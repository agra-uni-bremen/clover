#include <assert.h>
#include <stdlib.h>

#include <clover/clover.h>

using namespace clover;

#define CHECK_BRANCH(BRANCH, ...) \
	(BRANCH && BRANCH->getRandomPath(__VA_ARGS__))

Trace::Branch::Branch(bool _negated, std::shared_ptr<BitVector> _bv)
{
	negated = _negated;
	bv = _bv;

	true_branch = nullptr;
	false_branch = nullptr;
}

bool
Trace::Branch::getRandomPath(Path &path)
{
	assert(this->bv != nullptr);
	path.push_back(this->bv);

	/* XXX: This prefers node in the upper tree */
	if (!this->negated) {
		this->negated = true;
		return true;
	}

	/* Randomly traverse true or false branch first */
	int random = rand();
	if (random % 2 == 0)
		if (CHECK_BRANCH(true_branch, path) ||
		    CHECK_BRANCH(false_branch, path))
			return true;
	else
		if (CHECK_BRANCH(false_branch, path) ||
		    CHECK_BRANCH(true_branch, path))
			return true;


	path.pop_back(); // node is not on path
	return false;
}
