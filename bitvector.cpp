#include <clover/clover.h>

#include "fns.h"

using namespace clover;

BitVector::BitVector(const klee::ref<klee::Expr> &_expr)
    : expr(_expr)
{
	return;
}

BitVector::BitVector(IntValue value)
{
	size_t bytesize, bitsize;

	bytesize = intByteSize(value);
	bitsize = bytesize * 8;

	uint64_t exprValue = intToUint(value);
	this->expr = klee::ConstantExpr::create(exprValue, (unsigned)bitsize);
}

BitVector::BitVector(const klee::Array *array)
{
	unsigned bitsize;

	/* Array size is in bytes, needed for CreateArray(). However,
	 * createTempRead() requires a size in bits, calling this
	 * function is mandatory to convert the array to an expression. */
	bitsize = array->getSize() * 8;

	this->expr = klee::Expr::createTempRead(array, bitsize);
}
