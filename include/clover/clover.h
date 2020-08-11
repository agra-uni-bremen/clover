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
	BitVector(const klee::ref<klee::Expr> &expr);
	klee::ref<klee::Expr> expr;

public:
	BitVector(IntValue value);
	BitVector(const klee::Array *array);

	klee::Query toQuery(klee::ConstraintSet &cs);

	/* TODO: Autogenerate these methods using a macro? */
	std::shared_ptr<BitVector> add(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> slt(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> neg(void);

	/* TODO: remove this */
	friend class Trace;
};

/* TODO: Make use of std::optional for symbolic part */
class ConcolicValue {
public:
	std::shared_ptr<BitVector> concrete;
	std::shared_ptr<BitVector> symbolic; /* TODO: make this std::optional */

	/* TODO: Resolve duplication with BitVector class */
	/* TODO: Autogenerate these methods using a macro? */
	std::shared_ptr<ConcolicValue> add(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> slt(std::shared_ptr<ConcolicValue> other);

	bool hasSymbolic(void);
private:
	ConcolicValue(std::shared_ptr<BitVector> _concrete, std::shared_ptr<BitVector> _symbolic);
	ConcolicValue(std::shared_ptr<BitVector> _concrete);

	friend class Solver;
};

class Solver {
private:
	klee::Solver *solver;
	klee::ArrayCache array_cache;

public:
	Solver(klee::Solver *_solver = NULL);

	int eval(const klee::Query &query);
	int eval(std::shared_ptr<BitVector> bv);

	uint64_t evalValue(const klee::Query &query, unsigned bits = 64);
	uint64_t evalValue(std::shared_ptr<BitVector> bv, unsigned bits = 64);

	std::shared_ptr<ConcolicValue> BVC(std::optional<std::string> name, IntValue value);

	/* TODO: Provide a wrapper for getInitialValues and remove this */
	friend class Trace;
};

typedef std::map<std::string, IntValue> ConcreteStore;

class Trace {
private:
	std::vector<std::shared_ptr<BitVector>> pathCons;
	Solver &solver;

	size_t getRandomPathCond(void);
	std::optional<klee::Query> negateRandom(klee::ConstraintSet &cs);
	std::optional<klee::Assignment> generateNewAssign(void);

public:
	Trace(Solver &_solver);

	/* TODO: Add different methods for finding new variable assignments */
	/* TODO: Separete getStore() from path negation */

	void add(std::shared_ptr<BitVector> bv);
	std::optional<ConcreteStore> getStore(void);
};

};

#endif
