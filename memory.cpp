#include <clover/clover.h>

using namespace clover;

ConcolicMemory::ConcolicMemory(Solver &_solver)
	: solver(_solver)
{
	return;
}

void
ConcolicMemory::reset(void)
{
	data.clear();
}

std::shared_ptr<ConcolicValue>
ConcolicMemory::load(Addr addr, unsigned bytesize)
{
	std::shared_ptr<ConcolicValue> result = nullptr;
	for (uint32_t off = 0; off < bytesize; off++) {
		auto read_addr = addr + off;
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

std::shared_ptr<ConcolicValue>
ConcolicMemory::load(std::shared_ptr<ConcolicValue> addr, unsigned bytesize)
{
	auto base_addr = solver.evalValue<ConcolicMemory::Addr>(addr->concrete);
	return load(base_addr, bytesize);
}

void
ConcolicMemory::store(Addr addr, std::shared_ptr<ConcolicValue> value, unsigned bytesize)
{
	for (size_t off = 0; off < bytesize; off++) {
		// Extract expression works on bit indicies, not bytes.
		data[addr + off] = value->extract(off * 8, klee::Expr::Int8);
	}
}

void
ConcolicMemory::store(std::shared_ptr<ConcolicValue> addr, std::shared_ptr<ConcolicValue> value, unsigned bytesize)
{
	auto base_addr = solver.evalValue<ConcolicMemory::Addr>(addr->concrete);
	return store(base_addr, value, bytesize);
}
