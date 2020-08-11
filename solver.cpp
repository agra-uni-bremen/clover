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
Solver::BVC(std::string name, uint64_t size, int64_t value)
{
	auto array = array_cache.CreateArray(name, size);

	auto bvv = std::make_shared<BitVector>(value, size);
	auto bvs = std::make_shared<BitVector>(array);

	auto bvc = ConcolicValue(bvv, bvs);
	return std::make_shared<ConcolicValue>(bvc);
}

std::shared_ptr<ConcolicValue>
Solver::BVC(std::string name, uint64_t size)
{
	/* TODO: improve random number generation */
	return this->BVC(name, size, rand());
}

std::shared_ptr<ConcolicValue>
Solver::BVC(int64_t value, uint64_t size)
{
	auto bvv = std::make_shared<BitVector>(value, size);
	auto bvc = ConcolicValue(bvv);

	return std::make_shared<ConcolicValue>(bvc);
}
