#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <clover/clover.h>
#include <klee/Expr/Constraints.h>
#include <klee/Expr/ExprUtil.h>

#include "fns.h"

using namespace clover;

Trace::Trace(Solver &_solver)
    : solver(_solver), cm(cs)
{
	pathCondsRoot = std::make_shared<Branch>(Branch()); /* placeholder */
	pathCondsCurrent = nullptr;
}

void
Trace::reset(void)
{
	cs = klee::ConstraintSet();
	pathCondsCurrent = nullptr;
}

void
Trace::add(bool condition, std::shared_ptr<BitVector> bv)
{
	auto c = (condition) ? bv->eqTrue() : bv->eqFalse();
	cm.addConstraint(c->expr);

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
Trace::getQuery(std::shared_ptr<BitVector> bv)
{
	auto expr = cm.simplifyExpr(cs, bv->expr);
	return klee::Query(cs, expr);
}

klee::Query
Trace::newQuery(klee::ConstraintSet &cs, Branch::Path &path)
{
	size_t query_idx = path.size() - 1;
	auto cm = klee::ConstraintManager(cs);

	for (size_t i = 0; i < path.size(); i++) {
		auto bv = path.at(i).first;
		auto cond = path.at(i).second;

		auto bvcond = (cond) ? bv->eqTrue() : bv->eqFalse();
		if (i < query_idx) {
			cm.addConstraint(bvcond->expr);
			continue;
		}

		// This is the last expression on the path. By negating
		// it we can potentially discover a new path.
		auto expr = cm.simplifyExpr(cs, bvcond->expr);
		return klee::Query(cs, expr).negateExpr();
	}

	throw "unreachable";
}

std::optional<klee::Assignment>
Trace::findNewPath(void)
{
	std::optional<klee::Assignment> assign;

	do {
		klee::ConstraintSet cs;

		Branch::Path path;
		if (!pathCondsRoot->getRandomPath(path))
			return std::nullopt; /* all branches exhausted */

		auto query = newQuery(cs, path);
		assign = solver.getAssignment(query);
	} while (!assign.has_value()); /* loop until we found a sat assignment */

	assert(assign.has_value());
	return assign;
}

ConcreteStore
Trace::getStore(const klee::Assignment &assign)
{
	ConcreteStore store;
	for (auto const &b : assign.bindings) {
		auto array = b.first;
		auto value = b.second;

		std::string name = array->getName();
		store[name] = intFromVector(value);
	}

	return store;
}
