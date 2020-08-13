#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

#include <klee/Expr/ExprUtil.h>
#include <klee/Expr/Constraints.h>
#include <clover/clover.h>

#include "fns.h"

using namespace clover;

Trace::Trace(Solver &_solver)
		: solver(_solver)
{
	pathCondsRoot = nullptr;
	pathCondsCurrent = nullptr;
}

void
Trace::reset(void)
{
	pathCondsCurrent = nullptr;
}

void
Trace::add(bool condition, unsigned id, std::shared_ptr<BitVector> bv)
{
	auto branch = std::make_shared<Branch>(Branch(id, bv));
	if (pathCondsRoot == nullptr) { /* first added branch condition */
		pathCondsRoot = branch;
		goto ret;
	} else if (pathCondsCurrent == nullptr) { /* reexploring after reset() */
		branch = pathCondsRoot;
		goto ret;
	}

	if (prevCond) {
		if (pathCondsCurrent->true_branch) {
			branch = pathCondsCurrent->true_branch;
			goto ret; /* no new path found */
		}
		pathCondsCurrent->true_branch = branch;
	} else {
		if (pathCondsCurrent->false_branch) {
			branch = pathCondsCurrent->false_branch;
			goto ret; /* no new path found */
		}
		pathCondsCurrent->false_branch = branch;
	}

ret:
	if (!negatedConds.count(id))
		negatedConds[id] = false;
	prevCond = condition;
	pathCondsCurrent = branch;
}

klee::Query
Trace::getQuery(klee::ConstraintSet &cs, unsigned lastid)
{
	std::vector<std::shared_ptr<BitVector>> path;

	if (!pathCondsRoot->getPath(lastid, path))
		throw std::invalid_argument("invalid id");

	auto cm = klee::ConstraintManager(cs);
	size_t i;
	for (i = 0; i < path.size() - 1; i++)
		cm.addConstraint(path.at(i)->expr);

	auto bv = path.at(i);
	auto expr = cm.simplifyExpr(cs, bv->expr);

	// XXX: Can we extract the constraints from cm instead?
	return klee::Query(cs, expr);
}

std::optional<unsigned>
Trace::getUnnegatedId(void)
{
	std::vector<unsigned> unnegated_ids;

	for (auto elem : negatedConds) {
		auto id = elem.first;
		auto negated = elem.second;

		if (!negated)
			unnegated_ids.push_back(id);
	}

	if (unnegated_ids.empty())
		return std::nullopt; /* All conditions have been negated */

	int random = rand();
	size_t rindex = (unsigned)random % unnegated_ids.size();

	return unnegated_ids.at(rindex);
}

std::optional<klee::Assignment>
Trace::negateRandom(klee::ConstraintSet &cs)
{
	auto id = getUnnegatedId();
	if (!id.has_value())
		return std::nullopt;
	auto query = getQuery(cs, *id);

	auto assign = solver.getAssignment(query.negateExpr());
	if (!assign.has_value())
		return std::nullopt; /* unsat */

	return assign;
}

ConcreteStore
Trace::getStore(const klee::Assignment &assign)
{
	ConcreteStore store;
	for (auto const& b : assign.bindings) {
		auto array = b.first;
		auto value = b.second;

		std::string name = b.first->getName();
		store[name] = intFromVector(value);
	}

	return store;
}
