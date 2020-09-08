#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <clover/clover.h>
#include <klee/Expr/Constraints.h>
#include <klee/Expr/ExprUtil.h>

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

std::optional<klee::Query>
Trace::getQuery(klee::ConstraintSet &cs, Branch::Path &path)
{
	auto cm = klee::ConstraintManager(cs);

	size_t i;
	for (i = 0; i < path.size() - 1; i++) {
		auto bv = path.at(i).first;
		auto cond = path.at(i).second;

		// Adjust branch condition according to the path we are
		// taking (i.e. true or false branch), negate if false.
		auto bvcond = (cond) ? bv : bv->negate();

		// Adding constant unsatisfiable constraints causes a
		// failed assert in addConstraints, check these explicitly.
		auto expr = cm.simplifyExpr(cs, bvcond->expr);
		if (expr->getKind() == klee::Expr::Constant && !cast<klee::ConstantExpr>(expr)->isTrue())
			return std::nullopt;
		cm.addConstraint(expr);
	}

	auto bv = path.at(i).first;
	auto expr = cm.simplifyExpr(cs, bv->expr);

	// XXX: Can we extract the constraints from cm instead?
	return klee::Query(cs, expr);
}

std::optional<klee::Assignment>
Trace::findNewPath(klee::ConstraintSet &cs)
{
	std::optional<klee::Assignment> assign;

	do {
		Branch::Path path;
		if (!pathCondsRoot->getRandomPath(path))
			return std::nullopt; /* all branches exhausted */
		auto base_query = getQuery(cs, path);
		if (!base_query.has_value())
			continue; /* unsatisfiable constraint */

		// If the leaf branch condition was true in a previous run, we
		// are looking for an assignment so that it becomes false. If it
		// was false, we are looking for an assignment so that it
		// becomes true.
		auto leaf = path.at(path.size() - 1);
		auto query = (leaf.second) ? (*base_query).negateExpr() : *base_query;

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
