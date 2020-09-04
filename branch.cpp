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
Trace::Branch::getRandomPath(Path &path)
{
	if (this->isPlaceholder())
		return false;

	// Second part of pair is modified by reference later
	PathElement pair = std::make_pair(this->bv, false);
	path.push_back(pair);

	/* XXX: This prefers node in the upper tree */
	if (!wasNegated && (!this->true_branch || !this->false_branch)) {
		pair.second = (this->true_branch != nullptr);
		if (pair.second)
			assert(this->false_branch == nullptr);

		wasNegated = true;
		return true; /* Found undiscovered path */
	}

	/* Randomly traverse true or false branch first */
	int random = rand();
	if (random % 2 == 0) {
		if (CHECK_BRANCH(true_branch, path)) {
			pair.second = true;
			return true;
		} else if (CHECK_BRANCH(false_branch, path)) {
			pair.second = false;
			return true;
		}
	} else {
		if (CHECK_BRANCH(false_branch, path)) {
			pair.second = false;
			return true;
		} else if (CHECK_BRANCH(true_branch, path)) {
			pair.second = true;
			return true;
		}
	}

	path.pop_back(); // node is not on path
	return false;
}
