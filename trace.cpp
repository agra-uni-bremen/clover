#include <stdlib.h>
#include <stddef.h>

#include <clover/clover.h>

using namespace clover;

Trace::Trace(Solver &_solver)
		: solver(_solver)
{
	return;
}

void
Trace::negateRandom(void)
{
	int random = rand();
	size_t idx = (unsigned)random % this->pathCons.size();

	pathCons.at(idx) = pathCons.at(idx)->neg();
}

void
Trace::add(std::shared_ptr<BitVector> bv)
{
	pathCons.push_back(bv);
	return;
}
