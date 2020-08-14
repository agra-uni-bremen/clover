#include <klee/Expr/ExprBuilder.h>
#include <clover/clover.h>

#include "fns.h"

using namespace clover;

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

std::shared_ptr<BitVector>
BitVector::read(klee::UpdateList &ul, unsigned index)
{
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Read(ul, klee::ConstantExpr::create(index, klee::Expr::Int32));

	auto bv = BitVector(expr);
	return std::make_shared<BitVector>(bv);
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
BitVector::concat(std::shared_ptr<BitVector> other)
{
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Concat(this->expr, other->expr);

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
