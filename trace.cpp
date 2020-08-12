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
	prevCond = false;
}

void
Trace::add(bool condition, unsigned id, std::shared_ptr<BitVector> bv)
{

	/* TODO: handle case where we have discovered a new false/true branch */

	Branch &branch = pathCondsRoot;
	if (!branch.bv.has_value()) {
		branch.id = id;
		branch.bv = bv;
		goto ret;
	}

	branch = Branch(id, bv);
	if (prevCond) {
		assert(!pathCondsCurrent.true_branch.has_value());
		pathCondsCurrent.true_branch = branch;
	else {
		assert(!pathCondsCurrent.false_branch.has_value());
		pathCondsCurrent.false_branch = branch;
	}

ret:
	if (negatedConds.count(id))
		throw std::invalid_argument("id is not unique");

	negatedConds[id] = false;
	prevCond = condition;
	pathCondsCurrent = branch;
}

klee::Query
Trace::getQuery(klee::ConstraintSet &cs, unsigned lastid)
{
	std::vector<std::shared_ptr<BitVector>> path;

	if (!pathCondsRoot.getPath(lastid, path))
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
	auto query = getQuery(cs, id);

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
