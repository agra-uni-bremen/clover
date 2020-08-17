#include <stdlib.h>

#include <iostream>

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

		std::shared_ptr<ConcolicValue> byte = nullptr;
		try {
			byte = data.at(concrete_addr);
		} catch (const std::out_of_range&) {
			std::cerr << "Making memory at " << "0x" << std::hex << concrete_addr << " unconstrained symbolic" << std::endl;
			IntValue random = (uint32_t)rand();
			byte = solver.BVC("memory_byte", random);
		}

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
	for (uint32_t i = 0; i < bytesize; i++) {
		IntValue off = (uint32_t)i;

		auto write_addr = addr->add(solver.BVC(std::nullopt, off));
		auto concrete_addr = solver.evalValue<ConcolicMemory::Addr>(write_addr->concrete);

		data[concrete_addr] = value->extract(i, klee::Expr::Int8);
	}
}
