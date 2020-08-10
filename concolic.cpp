#include <clover/clover.h>

using namespace clover;

ConcolicValue::ConcolicValue(std::shared_ptr<BitVector> _concrete, std::shared_ptr<BitVector> _symbolic)
		: concrete(_concrete), symbolic(_symbolic)
{
	return;
}

ConcolicValue::ConcolicValue(std::shared_ptr<BitVector> _concrete)
		: concrete(_concrete), symbolic(nullptr)
{
	return;
}

std::shared_ptr<ConcolicValue>
ConcolicValue::add(std::shared_ptr<ConcolicValue> other)
{
	auto bvv = concrete->add(other->concrete);

	std::shared_ptr<BitVector> bvs_this = (this->hasSymbolic()) ?
		this->symbolic : this->concrete;
	std::shared_ptr<BitVector> bvs_other = (other->hasSymbolic()) ?
		other->symbolic : other->concrete;

	if (this->hasSymbolic() || other->hasSymbolic()) {
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

	std::shared_ptr<BitVector> bvs_this = (this->hasSymbolic()) ?
		this->symbolic : this->concrete;
	std::shared_ptr<BitVector> bvs_other = (other->hasSymbolic()) ?
		other->symbolic : other->concrete;

	if (this->hasSymbolic() || other->hasSymbolic()) {
		auto bvs = bvs_this->slt(bvs_other);
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv, bvs));
	} else {
		return std::make_shared<ConcolicValue>(ConcolicValue(bvv));
	}
}

bool
ConcolicValue::hasSymbolic(void) {
	return this->symbolic != nullptr;
}
