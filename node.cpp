#include <assert.h>
#include <stdlib.h>

#include <clover/clover.h>

using namespace clover;

Trace::Node::Node(void)
{
	value = nullptr;

	true_branch = nullptr;
	false_branch = nullptr;
}

bool
Trace::Node::isPlaceholder(void)
{
	return this->value == nullptr;
}
