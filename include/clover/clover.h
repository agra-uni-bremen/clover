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

namespace clover {

class BitVector {
private:
	BitVector(const klee::ref<klee::Expr> &expr);

public:
	klee::ref<klee::Expr> expr;

	BitVector(int64_t value, uint64_t size);
	BitVector(const klee::Array *array);

	std::shared_ptr<BitVector> add(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> slt(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> neg(void);

	void dump(void);

	/* TODO: add to_query method and remove friend class */
	friend class Solver;
};

class ConcolicValue {
public:
	std::shared_ptr<BitVector> concrete;
	std::shared_ptr<BitVector> symbolic;

	/* TODO: Modify this instead of returning new value? */
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

	std::shared_ptr<BitVector> BVS(std::string name, uint64_t size);
	std::shared_ptr<BitVector> BVV(int64_t value, uint64_t size);

	std::shared_ptr<ConcolicValue> BVC(std::string name, uint64_t size, int64_t value);
	std::shared_ptr<ConcolicValue> BVC(std::string name, uint64_t size);
	std::shared_ptr<ConcolicValue> BVC(int64_t value, uint64_t size);

	/* TODO: Provide a wrapper for getInitialValues and remove this */
	friend class Trace;
};

class Trace {
private:
	std::vector<std::shared_ptr<BitVector>> pathCons;
	Solver &solver;

	size_t getRandomPathCond(void);
	std::optional<klee::Query> negateRandom(klee::ConstraintSet &cs);
	std::optional<klee::Assignment> generateNewAssign(void);

public:
	Trace(Solver &_solver);

	void add(std::shared_ptr<BitVector> bv);
	bool getStore(void);
};

};

#endif
