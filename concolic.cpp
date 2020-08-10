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
	auto bvs = symbolic->add(other->symbolic);

	auto bvc = ConcolicValue(bvv, bvs);
	return std::make_shared<ConcolicValue>(bvc);
}

bool
ConcolicValue::hasSymbolic(void) {
	return this->symbolic == nullptr;
}
