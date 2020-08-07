#include <klee/Expr/ExprBuilder.h>
#include <clover/clover.h>

static unsigned
toBits(unsigned size)
{
    return size * 8;
}

XBitVector::XBitVector(const klee::ref<klee::Expr> &_expr)
		: expr(_expr)
{
	return;
}

XBitVector::XBitVector(int64_t value, uint64_t size)
{
	unsigned bitsize;

	/* Passed size in bytes, we need to convert it to bits. */
	bitsize = toBits(size);

	this->expr = klee::ConstantExpr::create(value, bitsize);
}

XBitVector::XBitVector(const klee::Array *array)
{
	unsigned bitsize;

	/* Array size is in bytes, needed for CreateArray(). However,
	 * createTempRead() requires a size in bits, calling this
	 * function is mandatory to convert the array to an expression. */
	bitsize = toBits(array->getSize());

	this->expr = klee::Expr::createTempRead(array, bitsize);
}

std::shared_ptr<XBitVector>
XBitVector::add(std::shared_ptr<XBitVector> other) {
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Add(this->expr, other->expr);

	auto bv = XBitVector(expr);
	return std::make_shared<XBitVector>(bv);
}

std::shared_ptr<XBitVector>
XBitVector::slt(std::shared_ptr<XBitVector> other) {
	auto exb = klee::createDefaultExprBuilder();
	auto expr = exb->Slt(this->expr, other->expr);

	auto bv = XBitVector(expr);
	return std::make_shared<XBitVector>(bv);
}
