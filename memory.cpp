#include <clover/clover.h>

using namespace clover;

ConcolicMemory::ConcolicMemory(Solver &_solver)
	: solver(_solver)
{
	return;
}

std::shared_ptr<ConcolicValue>
ConcolicMemory::load(std::shared_ptr<ConcolicValue> addr, unsigned bytesize)
{
	std::shared_ptr<ConcolicValue> result = nullptr;
	for (uint32_t i = 0; i < bytesize; i++) {
		IntValue off = (uint32_t)i;

		auto read_addr = addr->add(solver.BVC(std::nullopt, off));
		auto concrete_addr = solver.evalValue<ConcolicMemory::Addr>(read_addr->concrete);
		if (!data.count(concrete_addr)) {
			throw std::range_error("access to uninitialized memory"); // TODO
		}

		auto byte = data.at(concrete_addr);
		if (!result) {
			result = byte;
		} else {
			result->concat(byte);
		}
	}

	return result;
}

void
ConcolicMemory::store(std::shared_ptr<ConcolicValue> addr, std::shared_ptr<ConcolicValue> value, unsigned bytesize)
{
	// This requires converting a ConstantExpr (and an Array
	// encapsulated in a ReadLSB) back to an Array.
	//
	// Maybe using ArrayCache::CreateArray with ConstantValuesBegin argument?
	klee::UpdateList ul(array, 0);

	for (uint32_t i = 0; i < bytesize; i++) {
		IntValue off = (uint32_t)i;

		auto write_addr = addr->add(solver.BVC(std::nullopt, off));
		auto concrete_addr = solver.evalValue<ConcolicMemory::Addr>(write_addr->concrete);

		data[concrete_addr] = value->read(ul, i);
	}
}
