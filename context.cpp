#include <assert.h>
#include <stdlib.h>

#include <clover/clover.h>

using namespace clover;

ExecutionContext::ExecutionContext(Solver &_solver)
    : solver(_solver)
{
	return;
}

ConcreteStore
ExecutionContext::getPrevStore(void)
{
	if (last_run.empty())
		throw std::invalid_argument("assignment for last run is empty");

	return last_run;
}

bool
ExecutionContext::setupNewValues(ConcreteStore store)
{
	for (auto assign : store) {
		std::string name = assign.first;
		IntValue value = assign.second;

		/* Cache value for next invocation of getSymbolic() */
		next_run[name] = value;
	}

	last_run.clear(); // Clear variable assignment of last run
	return true;
}

bool
ExecutionContext::setupNewValues(Trace &trace)
{
	auto assign = trace.findNewPath();
	if (!assign.has_value())
		return false;

	return setupNewValues(trace.getStore(*assign));
}

std::shared_ptr<ConcolicValue>
ExecutionContext::getSymbolicWord(std::string name)
{
	IntValue concrete = findRemoveOrRandom<uint32_t>(name);
	return solver.BVC(name, concrete);
}

/* TODO: Possible optimization: Assume that memory passed to this
 * function does not overlap and store the value directly in the map
 * without splitting it into single bytes as this split is already
 * done by the ConcolicMemory::store function */
std::shared_ptr<ConcolicValue>
ExecutionContext::getSymbolicBytes(std::string name, size_t size)
{
	std::shared_ptr<ConcolicValue> result = nullptr;

	for (size_t i = 0; i < size; i++) {
		std::string bname = name + "byte" + std::to_string(i);

		IntValue concrete = findRemoveOrRandom<uint8_t>(bname);
		auto symbyte = solver.BVC(bname, concrete); /* TODO: eternal=false? */

		if (!result) {
			result = symbyte;
		} else {
			result = symbyte->concat(result);
		}
	}

	return result;
}
