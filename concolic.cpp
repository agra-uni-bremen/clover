#include <clover/clover.h>

using namespace clover;

ConcolicValue::ConcolicValue(std::shared_ptr<BitVector> _concrete, std::shared_ptr<BitVector> _symbolic)
		: concrete(_concrete), symbolic(_symbolic)
{
	return;
}

ConcolicValue::ConcolicValue(std::shared_ptr<BitVector> _concrete)
		: concrete(_concrete), symbolic(std::nullopt)
{
	return;
}

std::shared_ptr<ConcolicValue>
ConcolicValue::add(std::shared_ptr<ConcolicValue> other)
{
	auto bvv = concrete->add(other->concrete);

	auto bvs_this = this->symbolic.value_or(this->concrete);
	auto bvs_other = other->symbolic.value_or(other->concrete);

	if (this->symbolic.has_value() || other->symbolic.has_value()) {
		auto bvs = bvs_this->add(bvs_other);
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}

std::shared_ptr<ConcolicValue>
ConcolicValue::slt(std::shared_ptr<ConcolicValue> other)
{
	auto bvv = concrete->slt(other->concrete);

	auto bvs_this = this->symbolic.value_or(this->concrete);
	auto bvs_other = other->symbolic.value_or(other->concrete);

	if (this->symbolic.has_value() || other->symbolic.has_value()) {
		auto bvs = bvs_this->slt(bvs_other);
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}
