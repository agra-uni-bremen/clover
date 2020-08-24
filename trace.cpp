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
	pathCondsRoot = std::make_shared<Branch>(Branch()); /* placeholder */
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
	std::shared_ptr<Branch> branch = nullptr;
	if (pathCondsCurrent != nullptr) {
		branch = pathCondsCurrent;
	} else {
		branch = pathCondsRoot;
	}

	assert(branch);
	if (branch->isPlaceholder())
		branch->bv = bv;

	if (condition) {
		if (!branch->true_branch)
			branch->true_branch = std::make_shared<Branch>(Branch());
		pathCondsCurrent = branch->true_branch;
	} else {
		if (!branch->false_branch)
			branch->false_branch = std::make_shared<Branch>(Branch());
		pathCondsCurrent = branch->false_branch;
	}
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
Trace::findNewPath(klee::ConstraintSet &cs)
{
	Branch::Path path;
	bool wastrue;

	if (!pathCondsRoot->getRandomPath(path, wastrue))
		return std::nullopt;
	auto base_query = getQuery(cs, path);

	// If the branch condition was true in a previous run, we are
	// looking for an assignment so that it becomes false. If it was
	// false, we are looking for an assignment so that it becomes true.
	auto query = (wastrue) ? base_query.negateExpr() : base_query;

	auto assign = solver.getAssignment(query);
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

		std::string name = array->getName();
		store[name] = intFromVector(value);
	}

	return store;
}
