#include <assert.h>
#include <stdlib.h>

#include <regex>

#include <clover/clover.h>

using namespace clover;

static std::optional<size_t>
parseRegister(std::string name)
{
	if (name.length() < 2 || name.at(0) != 'x')
		return std::nullopt;

	auto idx = std::stoi(name.substr(1));
	return idx;
}

static std::optional<std::pair<Context::Address, size_t>>
parseMemory(std::string name)
{
	std::regex re("memory<(.*),(.*)>");

	std::smatch match;
	if (regex_search(name, match, re)) {
		if (match.size() != 3) /* match includes the entire string */
			return std::nullopt;

		auto addr = std::stoi(match[1].str());
		auto size = std::stoi(match[2].str());

		return std::make_pair((Context::Address)addr, (size_t)size);
	}

	return std::nullopt;
}

Context::Context(Solver &_solver) : solver(_solver)
{
	return;
}

bool
Context::hasNewPath(Trace &trace)
{
	klee::ConstraintSet cs;
	auto assign = trace.findNewPath(cs);
	if (!assign.has_value())
		return false;

	auto store = trace.getStore(*assign);
	for (auto assign : store) {
		std::string name = assign.first;
		IntValue value = assign.second;

		auto ridx = parseRegister(name);
		if (ridx.has_value()) {
			registers[*ridx] = value;
		} else { /* Either register OR memory */
			auto mem = parseMemory(name);
			assert(mem.has_value());

			auto addr = (*mem).first;
			auto size = (*mem).second;

			assert(size == 1);
			memory[addr] = value;
		}
	}

	return true;
}

IntValue
Context::findRemoveOrRandom(std::map<size_t, IntValue> &map, size_t key)
{
	IntValue concrete;

	auto iter = map.find(key);
	if (iter != map.end()) {
		concrete = (*iter).second;
		map.erase(iter);
	} else {
		concrete = (uint32_t)rand();
	}

	return concrete;
}

std::shared_ptr<ConcolicValue>
Context::getSymbolic(size_t reg)
{
	IntValue concrete = findRemoveOrRandom(registers, reg);
	return solver.BVC("x" + std::to_string(reg), concrete);
}

std::shared_ptr<ConcolicValue>
Context::getSymbolic(Address addr, size_t len)
{
	std::shared_ptr<ConcolicValue> result = nullptr;
	for (size_t i = 0; i < len; i++) {
		Address byte_addr = addr + i;
		unsigned byte_size = 1;

		std::string vname = std::string("memory") + "<" + std::to_string(byte_addr)
			+ "," + std::to_string(byte_size) + ">";

		IntValue concrete = findRemoveOrRandom(memory, byte_addr);
		auto symbyte = solver.BVC(vname, concrete);

		if (!result) {
			result = symbyte;
		} else {
			result = result->concat(symbyte);
		}
	}

	return result;
}
