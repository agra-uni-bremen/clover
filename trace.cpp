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
	int random = rand();
	return (unsigned)random % this->pathCons.size();
}

void
Trace::add(std::shared_ptr<BitVector> bv)
{
	pathCons.push_back(bv);
	return;
}

klee::Query
Trace::negateRandom(void)
{
	klee::ConstraintSet cs;
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
	auto query = negateRandom();
	std::vector<const klee::Array *> objects;

	klee::findSymbolicObjects(query.expr, objects);
	for (auto e : query.constraints)
		klee::findSymbolicObjects(e, objects);

	std::vector<std::vector<unsigned char> > values;
	if (!solver.solver->getInitialValues(query, objects, values))
		return {}; /* unsat */

	return klee::Assignment(objects, values);
}
