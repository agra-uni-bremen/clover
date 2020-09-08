#include <assert.h>
#include <stdlib.h>

#include <regex>

#include <clover/clover.h>

using namespace clover;

ExecutionContext::ExecutionContext(Solver &_solver)
    : solver(_solver)
{
	return;
}

bool
ExecutionContext::hasNewPath(Trace &trace)
{
	klee::ConstraintSet cs;
	auto assign = trace.findNewPath(cs);
	if (!assign.has_value())
		return false;

	auto store = trace.getStore(*assign);
	for (auto assign : store) {
		std::string name = assign.first;
		IntValue value = assign.second;

		names[name] = value;
	}

	return true;
}

std::shared_ptr<ConcolicValue>
ExecutionContext::getSymbolic(size_t reg)
{
	std::string name = "x" + std::to_string(reg);

	IntValue concrete = findRemoveOrRandom<uint32_t>(name);
	return solver.BVC(name, concrete);
}

/* TODO: Possible optimization: Assume that memory passed to this
 * function does not overlap and store the value directly in the map
 * without splitting it into single bytes as this split is already
 * done by the ConcolicMemory::store function */
std::shared_ptr<ConcolicValue>
ExecutionContext::getSymbolic(Address addr, size_t len)
{
	std::shared_ptr<ConcolicValue> result = nullptr;
	for (size_t i = 0; i < len; i++) {
		Address byte_addr = addr + i;
		std::string vname = std::string("memory") + "<" + std::to_string(byte_addr) + ">";

		IntValue concrete = findRemoveOrRandom<uint8_t>(vname);
		auto symbyte = solver.BVC(vname, concrete); /* TODO: eternal=false? */

		if (!result) {
			result = symbyte;
		} else {
			result = symbyte->concat(result);
		}
	}

	return result;
}
