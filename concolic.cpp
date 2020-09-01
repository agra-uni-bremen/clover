#include <clover/clover.h>

using namespace clover;

#define BINARY_OPERATOR(NAME, FN)                                                                 \
	std::shared_ptr<ConcolicValue>                                                            \
	NAME(std::shared_ptr<ConcolicValue> other)                                                \
	{                                                                                         \
		auto expr = builder->FN(concrete->expr, other->concrete->expr);                   \
		auto bvv = std::make_shared<BitVector>(BitVector(expr));                          \
                                                                                                  \
		if (this->symbolic.has_value() || other->symbolic.has_value()) {                  \
			auto bvs_this = this->symbolic.value_or(this->concrete);                  \
			auto bvs_other = other->symbolic.value_or(other->concrete);               \
                                                                                                  \
			auto expr = builder->FN(bvs_this->expr, bvs_other->expr);                 \
			auto bvs = std::make_shared<BitVector>(BitVector(expr));                  \
                                                                                                  \
			return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv, bvs)); \
		} else {                                                                          \
			return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv));      \
		}                                                                                 \
	}

ConcolicValue::ConcolicValue(klee::ExprBuilder *_builder, std::shared_ptr<BitVector> _concrete, std::optional<std::shared_ptr<BitVector>> _symbolic)
    : concrete(_concrete), symbolic(_symbolic), builder(_builder)
{
	return;
}

BINARY_OPERATOR(ConcolicValue::eq, Eq)
BINARY_OPERATOR(ConcolicValue::ne, Ne)
BINARY_OPERATOR(ConcolicValue::lshl, Shl)
BINARY_OPERATOR(ConcolicValue::lshr, LShr)
BINARY_OPERATOR(ConcolicValue::ashr, AShr)
BINARY_OPERATOR(ConcolicValue::add, Add)
BINARY_OPERATOR(ConcolicValue::sub, Sub)
BINARY_OPERATOR(ConcolicValue::slt, Slt)
BINARY_OPERATOR(ConcolicValue::sge, Sge)
BINARY_OPERATOR(ConcolicValue::ult, Ult)
BINARY_OPERATOR(ConcolicValue::uge, Uge)
BINARY_OPERATOR(ConcolicValue::band, And)
BINARY_OPERATOR(ConcolicValue::bor, Or)
BINARY_OPERATOR(ConcolicValue::bxor, Xor)
BINARY_OPERATOR(ConcolicValue::concat, Concat)

std::shared_ptr<ConcolicValue>
ConcolicValue::bnot(void)
{
	auto expr = builder->Not(concrete->expr);
	auto bvv = std::make_shared<BitVector>(BitVector(expr));

	if (this->symbolic.has_value()) {
		auto expr = builder->Not((*symbolic)->expr);
		auto bvs = std::make_shared<BitVector>(BitVector(expr));

		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::extract(unsigned offset, klee::Expr::Width width)
{
	auto expr = builder->Extract(concrete->expr, offset, width);
	auto bvv = std::make_shared<BitVector>(BitVector(expr));

	if (this->symbolic.has_value()) {
		auto expr = builder->Extract((*symbolic)->expr, offset, width);
		auto bvs = std::make_shared<BitVector>(BitVector(expr));

		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::sext(klee::Expr::Width width)
{
	auto expr = builder->SExt(concrete->expr, width);
	auto bvv = std::make_shared<BitVector>(BitVector(expr));

	if (this->symbolic.has_value()) {
		auto expr = builder->SExt((*symbolic)->expr, width);
		auto bvs = std::make_shared<BitVector>(BitVector(expr));

		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::zext(klee::Expr::Width width)
{
	auto expr = builder->ZExt(concrete->expr, width);
	auto bvv = std::make_shared<BitVector>(BitVector(expr));

	if (this->symbolic.has_value()) {
		auto expr = builder->ZExt((*symbolic)->expr, width);
		auto bvs = std::make_shared<BitVector>(BitVector(expr));

		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(builder, bvv));
	}
}
