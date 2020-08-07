#include <clover/clover.h>

using namespace clover;

ConcolicValue::ConcolicValue(BitVector _concrete, BitVector _symbolic)
		: concrete(_concrete), symbolic(_symbolic)
{
	return;
}
