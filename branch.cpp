#include <assert.h>

#include <clover/clover.h>

using namespace clover;

Branch::Branch(unsigned id, std::shared_ptr<BitVector> bv)
{
	true_branch == nullptr;
	false_branch == nullptr;
}

bool
Branch::getPath(unsigned id, std::vector<std::shared_ptr<BitVector>> &path)
{
	assert(this->bv != nullptr);
	path.push_back(this->bv);

	if (this->id == id)
		return true;

	if ((true_branch && true_branch->getPath(id, path)) ||
	    (false_branch && false_branch->getPath(id, path)))
		return true;

	path.pop_back(); // this is not on path to id
	return false;
}
