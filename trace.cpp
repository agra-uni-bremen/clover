#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <clover/clover.h>
#include <klee/Expr/Constraints.h>
#include <klee/Expr/ExprUtil.h>

#include "fns.h"

#define CHECK_BRANCH(BRANCH, ...) \
	(BRANCH && randPathPreferHigh(BRANCH, __VA_ARGS__))

using namespace clover;

Trace::Trace(Solver &_solver)
    : solver(_solver), cm(cs)
{
	pathCondsRoot = std::make_shared<Node>(Node()); /* placeholder */
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

	std::shared_ptr<Node> node = nullptr;
	if (pathCondsCurrent != nullptr) {
		node = pathCondsCurrent;
	} else {
		node = pathCondsRoot;
	}

	assert(node);
	if (node->isPlaceholder())
		node->value = std::make_shared<Branch>(Branch(bv, false));

	if (condition) {
		if (!node->true_branch)
			node->true_branch = std::make_shared<Node>(Node());
		pathCondsCurrent = node->true_branch;
	} else {
		if (!node->false_branch)
			node->false_branch = std::make_shared<Node>(Node());
		pathCondsCurrent = node->false_branch;
	}
}

klee::Query
Trace::getQuery(std::shared_ptr<BitVector> bv)
{
	auto expr = cm.simplifyExpr(cs, bv->expr);
	return klee::Query(cs, expr);
}

klee::Query
Trace::newQuery(klee::ConstraintSet &cs, Path &path)
{
	size_t query_idx = path.size() - 1;
	auto cm = klee::ConstraintManager(cs);

	for (size_t i = 0; i < path.size(); i++) {
		auto bv = path.at(i).first->bv;
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

		Path path;
		if (!randPathPreferHigh(pathCondsRoot, path))
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

bool
Trace::randPathPreferHigh(std::shared_ptr<Node> node, Path &path)
{
	if (node->isPlaceholder())
		return false;

	// Second part of pair is modified by index later
	path.push_back(std::make_pair(node->value, false));
	size_t idx = path.size() - 1;

	/* XXX: This prefers node in the upper tree */
	auto v = node->value;
	if (!v->wasNegated && (!node->true_branch || !node->false_branch)) {
		path[idx].second = (node->true_branch != nullptr);
		if (path[idx].second)
			assert(node->false_branch == nullptr);

		v->wasNegated = true;
		return true; /* Found undiscovered path */
	}

	/* Randomly traverse true or false branch first */
	int random = rand();
	if (random % 2 == 0) {
		if (CHECK_BRANCH(node->true_branch, path)) {
			path[idx].second = true;
			return true;
		} else if (CHECK_BRANCH(node->false_branch, path)) {
			path[idx].second = false;
			return true;
		}
	} else {
		if (CHECK_BRANCH(node->false_branch, path)) {
			path[idx].second = false;
			return true;
		} else if (CHECK_BRANCH(node->true_branch, path)) {
			path[idx].second = true;
			return true;
		}
	}

	path.pop_back(); // node is not on path
	return false;
}
