#include <assert.h>
#include <stdlib.h>

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
	return this->eval(bv->toQuery(cs));
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
	return this->evalValue(bv->toQuery(cs), bits);
}

std::shared_ptr<ConcolicValue>
Solver::BVC(std::optional<std::string> name, IntValue value)
{
	auto concrete = std::make_shared<BitVector>(value);
	if (!name.has_value()) {
		auto concolic = ConcolicValue(concrete);
		return std::make_shared<ConcolicValue>(concolic);
	}

	auto array = array_cache.CreateArray(*name, getByteSize(value));
	auto symbolic = std::make_shared<BitVector>(array);

	auto concolic = ConcolicValue(concrete, symbolic);
	return std::make_shared<ConcolicValue>(concolic);
}
