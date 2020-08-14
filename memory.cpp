#include <clover/clover.h>

using namespace clover;

ConcolicMemory::ConcolicMemory(void)
{
	return;
}

std::shared_ptr<ConcolicValue>
ConcolicMemory::load(std::shared_ptr<ConcolicValue> addr, unsigned bytesize)
{
	throw "not implemented";
}

void
store(std::shared_ptr<ConcolicValue> addr, unsigned bytesize)
{
	throw "not implemented";
}
