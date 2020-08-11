#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include <klee/Expr/ExprUtil.h>
#include <klee/Expr/Constraints.h>
#include <clover/clover.h>

using namespace clover;

Trace::Trace(Solver &_solver)
		: solver(_solver)
{
	return;
}

size_t
Trace::getRandomPathCond(void)
{
	size_t size = this->pathCons.size();

	/* Modulo zero is undefined behaviour in C++ */
	assert(size > 0);

	int random = rand();
	return (unsigned)random % size;
}

void
Trace::add(std::shared_ptr<BitVector> bv)
{
	pathCons.push_back(bv);
	return;
}

std::optional<klee::Query>
Trace::negateRandom(klee::ConstraintSet &cs)
{
	if (this->pathCons.empty())
		return {};

	auto cm = klee::ConstraintManager(cs);

	size_t i = 0;
	size_t rindex = getRandomPathCond();
	for (i = 0; i < rindex; i++)
		cm.addConstraint(pathCons.at(i)->expr);

	auto bv = pathCons.at(i)->neg();
	auto expr = cm.simplifyExpr(cs, bv->expr);

	// XXX: Can we extract the constraints from cm instead?
	return klee::Query(cs, expr);
}

std::optional<klee::Assignment>
Trace::generateNewAssign(void)
{
	klee::ConstraintSet cs;

	auto query = negateRandom(cs);
	if (!query.has_value())
		return {};

	std::vector<const klee::Array *> objects;
	klee::findSymbolicObjects(query->expr, objects);
	for (auto e : query->constraints)
		klee::findSymbolicObjects(e, objects);

	std::vector<std::vector<unsigned char>> values;
	if (!solver.solver->getInitialValues(*query, objects, values))
		return {}; /* unsat */

	return klee::Assignment(objects, values);
}

IntValue
Trace::convertResult(std::vector<unsigned char> res)
{
	IntValue intval;

	switch (res.size()) {
	case 1: {
		uint8_t v;
		memcpy(&v, &res[0], sizeof(uint8_t));
		intval = v;
	}
		break;
	case 4: {
		uint32_t v;
		memcpy(&v, &res[0], sizeof(uint32_t));
		intval = v;
	}
		break;
	default:
		assert(0);
	}

	return intval;
}

std::optional<ConcreteStore>
Trace::getStore(void)
{
	auto assign = generateNewAssign();
	if (!assign.has_value())
		return {};

	/* XXX: We only support register values for now */
	int32_t store_value;

	ConcreteStore store;
	for (auto const& b : assign->bindings) {
		auto array = b.first;
		auto value = b.second;

		std::string name = b.first->getName();
		store[name] = convertResult(value);
	}

	return store;
}
