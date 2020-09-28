#include <fstream>
#include <iostream>
#include <regex>

#include <clover/clover.h>
using namespace clover;

typedef std::optional<std::pair<std::string, IntValue>> Assignment;
typedef enum {
	UINT8,
	UINT32,
} AssignType;

static std::optional<IntValue>
parseIntVal(AssignType type, std::string input)
{
	return (uint32_t)5; // TODO
}

static std::optional<AssignType>
parseAssignType(std::string type)
{
	return UINT32; // TODO
}

static Assignment
parseAssign(std::string assign)
{
	std::regex re("(..*)	(uint8_t|uint32_t)	([0-9][0-9]*)");

	std::smatch match;
	if (regex_search(assign, match, re)) {
		if (match.size() != 4) /* match includes the entire string */
			return std::nullopt;

		auto name = match[1].str();
		auto type = parseAssignType(match[2].str());
		if (!type.has_value())
			return std::nullopt;

		auto ival = parseIntVal(*type, match[3].str());
		if (!ival.has_value())
			return std::nullopt;

		return std::make_pair(name, *ival);
	}

	return std::nullopt;
}

ConcreteStore
TestCase::fromFile(std::string fileName)
{
	ConcreteStore assigns;

	std::ifstream file(fileName);
	if (!file.is_open()) {
		throw std::runtime_error("could not open " + fileName);
	}

	std::string line;
	size_t lineNum = 1;

	while (std::getline(file, line)) {
		Assignment assign = parseAssign(line);
		if (!assign.has_value()) {
			throw TestCase::ParserError(fileName, lineNum, "invalid assignment");
		}

		assigns[std::get<0>(*assign)] = std::get<1>(*assign);
		lineNum++;
	}

	file.close();
	return assigns;
}

void
TestCase::toFile(ConcreteStore store, std::string fileName)
{
	throw "not implemented";
}
