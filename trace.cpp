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
	return;
}

void
Trace::add(unsigned id, std::shared_ptr<BitVector> bv)
{
	/* XXX: This is just a temporary proof of concept, the
	 * underlying path constraint datastructure needs to be
	 * converted to a Tree anyhow. */
	for (auto pair : pathCons) {
		auto id = pair.first;
		if (id == id)
			return;
	}

	pathCons.push_back(std::make_pair(id, bv));
	return;
}

klee::Query
Trace::getQuery(klee::ConstraintSet &cs, size_t upto)
{
	if (upto >= pathCons.size())
		throw std::out_of_range("upto exceeds amount of path constraints");

	auto cm = klee::ConstraintManager(cs);
	size_t i;
	for (i = 0; i < upto; i++) {
		auto bv = std::get<1>(pathCons.at(i));
		cm.addConstraint(bv->expr);
	}

	auto bv = std::get<1>(pathCons.at(i));
	auto expr = cm.simplifyExpr(cs, bv->expr);

	// XXX: Can we extract the constraints from cm instead?
	return klee::Query(cs, expr);
}

ssize_t
Trace::getRandomIndex(void)
{
	/* modulo zero is undefined */
	assert(pathCons.size() > 0);

	int random = rand();
	size_t rindex = (unsigned)random % pathCons.size();

	if (!negatedCons.count(rindex)) {
		negatedCons[rindex] = true;
		return rindex;
	}

	/* Iterate through all path condition starting at ++rindex */
	size_t i = (rindex + 1) % pathCons.size();
	for (; i != rindex; i = (i + 1) % pathCons.size())
		if (!negatedCons.count(i))
			break;

	if (i == rindex)
		return -1; /* All conditions have been negated */

	negatedCons[i] = true;
	return i;
}

std::optional<klee::Assignment>
Trace::negateRandom(klee::ConstraintSet &cs)
{
	if (pathCons.empty())
		return std::nullopt;

	ssize_t rindex = getRandomIndex();
	if (rindex == -1)
		return std::nullopt;
	auto query = getQuery(cs, rindex);

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
