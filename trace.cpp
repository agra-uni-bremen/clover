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
Trace::add(bool condition, std::shared_ptr<BitVector> bv)
{
	auto branch = std::make_shared<Branch>(Branch(false, bv));
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
	prevCond = condition;
	pathCondsCurrent = branch;
}

klee::Query
Trace::getQuery(klee::ConstraintSet &cs, Branch::Path &path)
{
	auto cm = klee::ConstraintManager(cs);

	size_t i;
	for (i = 0; i < path.size() - 1; i++)
		cm.addConstraint(path.at(i)->expr);

	auto bv = path.at(i);
	auto expr = cm.simplifyExpr(cs, bv->expr);

	// XXX: Can we extract the constraints from cm instead?
	return klee::Query(cs, expr);
}

std::optional<klee::Assignment>
Trace::negateRandom(klee::ConstraintSet &cs)
{
	Branch::Path path;

	if (!pathCondsRoot->getRandomPath(path))
		return std::nullopt;
	auto query = getQuery(cs, path);

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
