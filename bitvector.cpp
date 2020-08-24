#include <clover/clover.h>

#include "fns.h"

using namespace clover;

#define BINARY_OPERATOR(NAME, FN)                                      \
	std::shared_ptr<BitVector>                                     \
	NAME(std::shared_ptr<BitVector> other) {                       \
		auto expr = builder-> FN (this->expr, other->expr);    \
		                                                       \
		auto bv = BitVector(builder, expr);                    \
		return std::make_shared<BitVector>(bv);                \
	}

static size_t
toBits(unsigned size)
{
    return size * 8;
}

BitVector::BitVector(klee::ExprBuilder *_builder, const klee::ref<klee::Expr> &_expr)
		: expr(_expr), builder(_builder)
{
	return;
}

BitVector::BitVector(klee::ExprBuilder *_builder, IntValue value)
{
	size_t bytesize, bitsize;

	bytesize = intByteSize(value);
	bitsize = toBits(bytesize);

	uint64_t exprValue = intToUint(value);

	this->expr = klee::ConstantExpr::create(exprValue, (unsigned)bitsize);
	this->builder = _builder;
}

BitVector::BitVector(klee::ExprBuilder *_builder, const klee::Array *array)
{
	unsigned bitsize;

	/* Array size is in bytes, needed for CreateArray(). However,
	 * createTempRead() requires a size in bits, calling this
	 * function is mandatory to convert the array to an expression. */
	bitsize = toBits(array->getSize());

	this->expr = klee::Expr::createTempRead(array, bitsize);
	this->builder = _builder;
}

BINARY_OPERATOR(BitVector::add, Add)
BINARY_OPERATOR(BitVector::slt, Slt)
BINARY_OPERATOR(BitVector::ult, Ult)
BINARY_OPERATOR(BitVector::uge, Uge)
BINARY_OPERATOR(BitVector::band, And)
BINARY_OPERATOR(BitVector::concat, Concat)

std::shared_ptr<BitVector>
BitVector::bnot(void)
{
	auto expr = builder->Not(this->expr);

	auto bv = BitVector(builder, expr);
	return std::make_shared<BitVector>(bv);
}


std::shared_ptr<BitVector>
BitVector::extract(unsigned offset, klee::Expr::Width width)
{
	auto expr = builder->Extract(this->expr, offset, width);

	auto bv = BitVector(builder, expr);
	return std::make_shared<BitVector>(bv);
}

std::shared_ptr<BitVector>
BitVector::sext(klee::Expr::Width width)
{
	auto expr = builder->SExt(this->expr, width);

	auto bv = BitVector(builder, expr);
	return std::make_shared<BitVector>(bv);
}
