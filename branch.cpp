#include <assert.h>

#include <clover/clover.h>

using namespace clover;

Branch::Branch(unsigned _id, std::shared_ptr<BitVector> _bv)
{
	id = _id;
	bv = _bv;

	true_branch == nullptr;
	false_branch == nullptr;
}

bool
Branch::getPath(unsigned id, Path &path)
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
