#include <assert.h>
#include <stdlib.h>

#include <clover/clover.h>

using namespace clover;

Trace::Branch::Branch(void)
{
	/* This is node is a placeholder */
	bv = nullptr;
	wasNegated = false;

	true_branch = nullptr;
	false_branch = nullptr;
}

bool
Trace::Branch::isPlaceholder(void)
{
	return this->bv == nullptr;
}
