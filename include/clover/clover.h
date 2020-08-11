#ifndef CLOVER_H
#define CLOVER_H

#include <stdint.h>
#include <stdbool.h>

#include <klee/Expr/Assignment.h>
#include <klee/Expr/ArrayCache.h>
#include <klee/Expr/Expr.h>
#include <klee/Solver/Solver.h>

#include <map>
#include <optional>
#include <variant>

namespace clover {

typedef std::variant<uint8_t, uint32_t> IntValue;

class BitVector {
private:
	klee::ref<klee::Expr> expr;

	BitVector(const klee::ref<klee::Expr> &expr);
	BitVector(IntValue value);
	BitVector(const klee::Array *array);

public:
	/* TODO: Autogenerate these methods using a macro? */
	std::shared_ptr<BitVector> add(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> slt(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> neg(void);

	friend class Solver;
	friend class Trace;
};

class ConcolicValue {
public:
	std::shared_ptr<BitVector> concrete;
	std::optional<std::shared_ptr<BitVector>> symbolic;

	/* TODO: Autogenerate these methods using a macro? */
	std::shared_ptr<ConcolicValue> add(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> slt(std::shared_ptr<ConcolicValue> other);
private:
	ConcolicValue(std::shared_ptr<BitVector> _concrete, std::optional<std::shared_ptr<BitVector>> _symbolic = std::nullopt);

	/* The solver acts as a factory for ConcolicValue */
	friend class Solver;
};

class Solver {
private:
	klee::Solver *solver;
	klee::ArrayCache array_cache;

public:
	Solver(klee::Solver *_solver = NULL);
	std::optional<klee::Assignment> getAssignment(const klee::Query &query);

	int eval(const klee::Query &query);
	int eval(std::shared_ptr<BitVector> bv);

	/* TODO: Return an IntVal here */
	uint64_t evalValue(const klee::Query &query, unsigned bits = 64);
	uint64_t evalValue(std::shared_ptr<BitVector> bv, unsigned bits = 64);

	std::shared_ptr<ConcolicValue> BVC(std::optional<std::string> name, IntValue value);
};

typedef std::map<std::string, IntValue> ConcreteStore;

class Trace {
private:
	std::vector<std::shared_ptr<BitVector>> pathCons;
	Solver &solver;

	klee::Query getQuery(klee::ConstraintSet &cs, size_t upto);

public:
	Trace(Solver &_solver);

	void add(std::shared_ptr<BitVector> bv);

	std::optional<klee::Assignment> negateRandom(klee::ConstraintSet &cs);
	ConcreteStore getStore(const klee::Assignment &assign);
};

};

#endif
