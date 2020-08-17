#include <assert.h>
#include <stdlib.h>

#include <clover/clover.h>

using namespace clover;

#define CHECK_BRANCH(BRANCH, ...) \
	(BRANCH && BRANCH->getRandomPath(__VA_ARGS__))

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

bool
Trace::Branch::getRandomPath(Path &path, bool &wastrue)
{
	if (this->isPlaceholder())
		return false;
	path.push_back(this->bv);

	/* XXX: This prefers node in the upper tree */
	if (!wasNegated) {
		if (!this->true_branch) {
			assert(this->false_branch);
			wastrue = false;

			wasNegated = true;
			return true; /* Found undiscovered path */
		} else if (!this->false_branch) {
			assert(this->true_branch);
			wastrue = true;

			wasNegated = true;
			return true; /* Found undiscovered path */
		}
	}

	/* Randomly traverse true or false branch first */
	int random = rand();
	if (random % 2 == 0)
		if (CHECK_BRANCH(true_branch, path, wastrue) ||
		    CHECK_BRANCH(false_branch, path, wastrue))
			return true;
	else
		if (CHECK_BRANCH(false_branch, path, wastrue) ||
		    CHECK_BRANCH(true_branch, path, wastrue))
			return true;

	path.pop_back(); // node is not on path
	return false;
}
