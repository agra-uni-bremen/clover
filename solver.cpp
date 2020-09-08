#include <assert.h>
#include <stdlib.h>

#include <clover/clover.h>

#include <klee/Expr/Constraints.h>
#include <klee/Expr/ExprUtil.h>

#include "fns.h"

using namespace clover;

Solver::Solver(klee::Solver *_solver)
{
	if (!_solver)
		_solver = klee::createCoreSolver(klee::CoreSolverType::STP_SOLVER);

	// Create fancy solver chain based on given core solver.
	// Taken from lib/Solver/ConstructSolverChain.cpp
	_solver = klee::createFastCexSolver(_solver);
	_solver = klee::createCexCachingSolver(_solver);
	_solver = klee::createCachingSolver(_solver);
	_solver = klee::createIndependentSolver(_solver);

	// Copied from tools/kleaver/main.cpp
	builder = klee::createDefaultExprBuilder();
	builder = createConstantFoldingExprBuilder(builder);
	builder = createSimplifyingExprBuilder(builder);

	this->solver = _solver;
	return;
}

Solver::~Solver(void)
{
	delete this->solver;
	delete this->builder;
}

std::optional<klee::Assignment>
Solver::getAssignment(const klee::Query &query)
{
	/* KLEE is concerned with validity of queries. To find a
	 * statisfiable assignment for a query it needs to be negated. */
	auto nq = query.negateExpr();

	std::vector<const klee::Array *> objects;
	klee::findSymbolicObjects(nq.expr, objects);
	for (auto e : nq.constraints)
		klee::findSymbolicObjects(e, objects);

	std::vector<std::vector<unsigned char>> values;
	if (!solver->getInitialValues(nq, objects, values))
		return std::nullopt; /* unsat */

	return klee::Assignment(objects, values);
}

bool
Solver::eval(const klee::Query &query)
{
	klee::Solver::Validity v;

	if (!solver->evaluate(query, v))
		throw std::runtime_error("solver failed to evaluate query");

	switch (v) {
	case klee::Solver::True:
		return true;
	case klee::Solver::False:
		return false;
	case klee::Solver::Unknown:
		throw std::logic_error("unknown solver result");
	}

	assert(0); /* unreachable */
}

bool
Solver::eval(std::shared_ptr<BitVector> bv)
{
	klee::ConstraintSet cs;

	auto q = klee::Query(cs, bv->expr);
	return this->eval(q);
}

std::shared_ptr<ConcolicValue>
Solver::BVC(std::optional<std::string> name, IntValue value, bool eternal)
{
	auto concrete = std::make_shared<BitVector>(BitVector(value));
	if (!name.has_value()) {
		auto concolic = ConcolicValue(builder, concrete);
		return std::make_shared<ConcolicValue>(concolic);
	}

	// The idea of eternal symbolic values was copied from angr. If
	// a variable is not eternalan incrementing counter will be
	// appended to the key to make the variable name unique.
	if (!eternal)
		(*name).append(std::to_string(varCounter++));

	auto array = array_cache.CreateArray(*name, intByteSize(value));
	auto symbolic = std::make_shared<BitVector>(BitVector(array));

	auto concolic = ConcolicValue(builder, concrete, symbolic);
	return std::make_shared<ConcolicValue>(concolic);
}
