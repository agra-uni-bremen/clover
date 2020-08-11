#include <klee/Expr/ExprBuilder.h>
#include <clover/clover.h>

using namespace clover;

static unsigned
toBits(unsigned size)
{
    return size * 8;
}

BitVector::BitVector(const klee::ref<klee::Expr> &_expr)
		: expr(_expr)
{
	return;
}

BitVector::BitVector(int64_t value, uint64_t size)
{
	unsigned bitsize;

	/* Passed size in bytes, we need to convert it to bits. */
	bitsize = toBits(size);

	this->expr = klee::ConstantExpr::create(value, bitsize);
}

BitVector::BitVector(const klee::Array *array)
{
	unsigned bitsize;

	/* Array size is in bytes, needed for CreateArray(). However,
	 * createTempRead() requires a size in bits, calling this
	 * function is mandatory to convert the array to an expression. */
	bitsize = toBits(array->getSize());

	this->expr = klee::Expr::createTempRead(array, bitsize);
}

klee::Query
BitVector::toQuery(klee::ConstraintSet &cs)
{
	return klee::Query(cs, this->expr);
}

std::shared_ptr<BitVector>
BitVector::add(std::shared_ptr<BitVector> other) {
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Add(this->expr, other->expr);

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
}

std::shared_ptr<BitVector>
BitVector::slt(std::shared_ptr<BitVector> other) {
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Slt(this->expr, other->expr);

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
}

std::shared_ptr<BitVector>
BitVector::neg(void)
{
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Not(this->expr);

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
}
