#include <clover/clover.h>

using namespace clover;

#define BINARY_OPERATOR(NAME, FN)                                                        \
	std::shared_ptr<ConcolicValue>                                                   \
	NAME(std::shared_ptr<ConcolicValue> other)                                       \
	{                                                                                \
		auto bvv = concrete->FN(other->concrete);                                \
                                                                                         \
		if (this->symbolic.has_value() || other->symbolic.has_value()) {         \
			auto bvs_this = this->symbolic.value_or(this->concrete);         \
			auto bvs_other = other->symbolic.value_or(other->concrete);      \
                                                                                         \
			auto bvs = bvs_this->FN(bvs_other);                              \
			return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs)); \
		} else {                                                                 \
			return std::make_shared<ConcolicValue>(ConcolicValue(bvv));      \
		}                                                                        \
	}

ConcolicValue::ConcolicValue(std::shared_ptr<BitVector> _concrete, std::optional<std::shared_ptr<BitVector>> _symbolic)
    : concrete(_concrete), symbolic(_symbolic)
{
	return;
}

BINARY_OPERATOR(ConcolicValue::eq, eq)
BINARY_OPERATOR(ConcolicValue::ne, ne)
BINARY_OPERATOR(ConcolicValue::lshl, lshl)
BINARY_OPERATOR(ConcolicValue::lshr, lshr)
BINARY_OPERATOR(ConcolicValue::ashr, ashr)
BINARY_OPERATOR(ConcolicValue::add, add)
BINARY_OPERATOR(ConcolicValue::sub, sub)
BINARY_OPERATOR(ConcolicValue::slt, slt)
BINARY_OPERATOR(ConcolicValue::sge, sge)
BINARY_OPERATOR(ConcolicValue::ult, ult)
BINARY_OPERATOR(ConcolicValue::uge, uge)
BINARY_OPERATOR(ConcolicValue::band, band)
BINARY_OPERATOR(ConcolicValue::bor, bor)
BINARY_OPERATOR(ConcolicValue::bxor, bxor)
BINARY_OPERATOR(ConcolicValue::concat, concat)

std::shared_ptr<ConcolicValue>
ConcolicValue::bnot(void)
{
	auto bvv = concrete->bnot();

	if (this->symbolic.has_value()) {
		auto bvs = (*symbolic)->bnot();
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::extract(unsigned offset, klee::Expr::Width width)
{
	auto bvv = concrete->extract(offset, width);

	if (this->symbolic.has_value()) {
		auto bvs = (*symbolic)->extract(offset, width);
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::sext(klee::Expr::Width width)
{
	auto bvv = concrete->sext(width);

	if (this->symbolic.has_value()) {
		auto bvs = (*symbolic)->sext(width);
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::zext(klee::Expr::Width width)
{
	auto bvv = concrete->zext(width);

	if (this->symbolic.has_value()) {
		auto bvs = (*symbolic)->zext(width);
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}
