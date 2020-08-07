#include <clover/clover.h>

#include <klee/Expr/Constraints.h>

using namespace clover;

Solver::Solver(klee::Solver *_solver)
{
	if (!_solver)
		_solver = klee::createCoreSolver(klee::CoreSolverType::STP_SOLVER);

	this->solver = _solver;
	return;
}

int
Solver::eval(const klee::Query &query)
{
	klee::Solver::Validity v;

	if (!solver->evaluate(query, v))
		throw std::runtime_error("solver failed to evaluate query");

	switch (v) {
	case klee::Solver::True:
		return 1;
	case klee::Solver::False:
		return 0;
	case klee::Solver::Unknown:
		return -1;
	}
}

int
Solver::eval(std::shared_ptr<BitVector> bv)
{
	klee::ConstraintSet cs;

	auto q = klee::Query(cs, bv->expr);
	return this->eval(q);
}

uint64_t
Solver::evalValue(const klee::Query &query, unsigned bits)
{
	klee::ref<klee::ConstantExpr> r;

	if (!solver->getValue(query, r))
		throw std::runtime_error("getValue() failed for solver");

	return r->getZExtValue(bits);
}

uint64_t
Solver::evalValue(std::shared_ptr<BitVector> bv, unsigned bits)
{
	klee::ConstraintSet cs;

	auto q = klee::Query(cs, bv->expr);
	return this->evalValue(q, bits);
}

std::shared_ptr<BitVector>
Solver::BVS(std::string name, uint64_t size)
{
	const klee::Array *array;

	array = array_cache.CreateArray(name, size);
	auto bv = BitVector(array);

	return std::make_shared<BitVector>(bv);
}

std::shared_ptr<BitVector>
Solver::BVV(int64_t value, uint64_t size)
{
	return std::make_shared<BitVector>(value, size);
}
