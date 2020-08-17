#include <klee/Expr/ExprBuilder.h>
#include <clover/clover.h>

#include "fns.h"

using namespace clover;

#define BINARY_OPERATOR(NAME, FN)                                      \
	std::shared_ptr<BitVector>                                     \
	NAME(std::shared_ptr<BitVector> other) {                       \
		auto exb = klee::createDefaultExprBuilder();           \
		auto expr = exb-> FN (this->expr, other->expr);        \
		                                                       \
		auto bv = BitVector(expr);                             \
		return std::make_shared<BitVector>(bv);                \
	}

static size_t
toBits(unsigned size)
{
    return size * 8;
}

BitVector::BitVector(const klee::ref<klee::Expr> &_expr)
		: expr(_expr)
{
	return;
}

BitVector::BitVector(IntValue value)
{
	size_t bytesize, bitsize;

	bytesize = intByteSize(value);
	bitsize = toBits(bytesize);

	uint64_t exprValue = intToUint(value);
	this->expr = klee::ConstantExpr::create(exprValue, (unsigned)bitsize);
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

BINARY_OPERATOR(BitVector::add, Add)
BINARY_OPERATOR(BitVector::slt, Slt)
BINARY_OPERATOR(BitVector::concat, Concat)

std::shared_ptr<BitVector>
BitVector::neg(void)
{
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Not(this->expr);

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
}

std::shared_ptr<BitVector>
BitVector::extract(unsigned offset, klee::Expr::Width width)
{
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Extract(this->expr, offset, width);

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
}

std::shared_ptr<BitVector>
BitVector::sext(klee::Expr::Width width)
{
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->SExt(this->expr, width);

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
}
