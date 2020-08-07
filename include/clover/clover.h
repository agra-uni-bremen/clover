#ifndef CLOVER_H
#define CLOVER_H

#include <stdint.h>

#include <klee/Expr/Expr.h>
#include <klee/Expr/ArrayCache.h>
#include <klee/Solver/Solver.h>

class XBitVector {
private:
	klee::ref<klee::Expr> expr;

	XBitVector(const klee::ref<klee::Expr> &expr);

public:
	XBitVector(int64_t value, uint64_t size);
	XBitVector(const klee::Array *array);

	std::shared_ptr<XBitVector> add(std::shared_ptr<XBitVector> other);
	std::shared_ptr<XBitVector> slt(std::shared_ptr<XBitVector> other);

	/* TODO: add to_query method and remove friend class */
	friend class XSolver;
};

class XSolver {
private:
	klee::Solver *solver;
	klee::ArrayCache array_cache;

public:
	XSolver(klee::Solver *_solver = NULL);

	int eval(const klee::Query &query);
	int eval(std::shared_ptr<XBitVector> bv);

	uint64_t evalValue(const klee::Query &query, unsigned bits = 64);
	uint64_t evalValue(std::shared_ptr<XBitVector> bv, unsigned bits = 64);

	std::shared_ptr<XBitVector> BVS(std::string name, uint64_t size);
	std::shared_ptr<XBitVector> BVV(int64_t value, uint64_t size);
};

#endif
