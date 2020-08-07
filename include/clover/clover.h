#ifndef CLOVER_H
#define CLOVER_H

#include <stdint.h>

#include <klee/Expr/Expr.h>
#include <klee/Expr/ArrayCache.h>
#include <klee/Solver/Solver.h>

namespace clover {

class BitVector {
private:
	klee::ref<klee::Expr> expr;

	BitVector(const klee::ref<klee::Expr> &expr);

public:
	BitVector(int64_t value, uint64_t size);
	BitVector(const klee::Array *array);

	std::shared_ptr<BitVector> add(std::shared_ptr<BitVector> other);
	std::shared_ptr<BitVector> slt(std::shared_ptr<BitVector> other);

	/* TODO: add to_query method and remove friend class */
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
};

};

#endif
