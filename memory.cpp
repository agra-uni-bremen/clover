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
	auto base_addr = solver.evalValue<ConcolicMemory::Addr>(addr->concrete);

	std::shared_ptr<ConcolicValue> result = nullptr;
	for (uint32_t off = 0; off < bytesize; off++) {
		auto read_addr = base_addr + off;
		if (!data.count(read_addr)) {
			throw std::range_error("access to uninitialized memory"); // TODO
		}

		auto byte = data.at(read_addr);
		if (!result) {
			result = byte;
		} else {
			result = result->concat(byte);
		}
	}

	return result;
}

void
ConcolicMemory::store(std::shared_ptr<ConcolicValue> addr, std::shared_ptr<ConcolicValue> value, unsigned bytesize)
{
	auto base_addr = solver.evalValue<ConcolicMemory::Addr>(addr->concrete);

	for (size_t off = 0; off < bytesize; off++) {
		// Extract expression works on bit indicies, not bytes.
		data[base_addr + off] = value->extract(off * 8, klee::Expr::Int8);
	}
}
