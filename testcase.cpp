#include <fstream>
#include <iostream>
#include <regex>

#include <inttypes.h>
#include <stdint.h>

#include <clover/clover.h>
using namespace clover;

typedef std::optional<std::pair<std::string, IntValue>> Assignment;
typedef enum {
	UINT8,
	UINT32,
} AssignType;

#define PARSE_INT(STR, FMT, TYPE)                    \
	{                                            \
		TYPE v;                              \
		int r = sscanf(STR, "%" FMT "", &v); \
                                                     \
		if (r == 1)                          \
			return (TYPE)v;              \
		else                                 \
			return std::nullopt;         \
	}

static std::optional<IntValue>
parseIntVal(AssignType type, std::string input)
{
	switch (type) {
	case UINT8:
		PARSE_INT(input.c_str(), SCNu8, uint8_t);
		break;
	case UINT32:
		PARSE_INT(input.c_str(), SCNu32, uint32_t);
		break;
	}

	return std::nullopt;
}

static std::optional<AssignType>
parseAssignType(std::string type)
{
	if (type == "uint8_t") {
		return UINT8;
	} else if (type == "uint32_t") {
		return UINT32;
	} else {
		return std::nullopt;
	}
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
