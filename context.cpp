#include <stdlib.h>

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

		auto reg_idx = parseRegister(name);
		if (!reg_idx.has_value())
			continue;

		registers[*reg_idx] = value;
	}

	return true;
}

std::shared_ptr<ConcolicValue>
Context::getSymbolic(size_t reg)
{
	IntValue concrete;

	auto iter = registers.find(reg);
	if (iter != registers.end()) {
		concrete = (*iter).second;
		registers.erase(iter);
	} else {
		concrete = (uint32_t)rand();
	}

	return solver.BVC("x" + std::to_string(reg), concrete);
}

std::shared_ptr<ConcolicValue>
Context::getSymbolic(Address addr, size_t len)
{
	return nullptr;
}
