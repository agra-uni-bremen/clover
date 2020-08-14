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
	std::shared_ptr<ConcolicValue> concat(std::shared_ptr<ConcolicValue> other);
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

	bool eval(const klee::Query &query);
	bool eval(std::shared_ptr<BitVector> bv);

	std::shared_ptr<ConcolicValue> BVC(std::optional<std::string> name, IntValue value);

	template <typename T> T evalValue(const klee::Query &query) {
		klee::ref<klee::ConstantExpr> r;

		if (!solver->getValue(query, r))
			throw std::runtime_error("getValue() failed for solver");

		return (T)r->getZExtValue(sizeof(T) * 8);
	}

	template <typename T> T evalValue(std::shared_ptr<BitVector> bv) {
		klee::ConstraintSet cs;

		auto q = klee::Query(cs, bv->expr);
		return this->evalValue<T>(q);
	}
};

class ConcolicMemory {
private:
	typedef uint32_t Addr;

	Solver &solver;
	std::unordered_map<Addr, std::shared_ptr<ConcolicValue>> data;

public:
	ConcolicMemory(Solver &_solver);

	std::shared_ptr<ConcolicValue> load(std::shared_ptr<ConcolicValue> addr, unsigned bytesize);
	void store(std::shared_ptr<ConcolicValue> addr, unsigned bytesize);
};

typedef std::map<std::string, IntValue> ConcreteStore;

class Trace {
private:
	class Branch {
	public:
		typedef std::vector<std::shared_ptr<BitVector>> Path;

		bool negated;
		std::shared_ptr<BitVector> bv;

		std::shared_ptr<Branch> true_branch;
		std::shared_ptr<Branch> false_branch;

		Branch(bool _negated, std::shared_ptr<BitVector> _bv);

		/* Get seemingly random path to an unnegated node in the tree.
		 * The node is also marked as negated before a path is returned. */
		bool getRandomPath(Path &path);
	};

	Solver &solver;

	std::shared_ptr<Branch> pathCondsRoot;
	std::shared_ptr<Branch> pathCondsCurrent;

	/* Whether the previsouly added branch was true or false */
	bool prevCond;

	klee::Query getQuery(klee::ConstraintSet &cs, Branch::Path &path);

public:
	Trace(Solver &_solver);
	void reset(void);

	void add(bool condition, std::shared_ptr<BitVector> bv);

	std::optional<klee::Assignment> negateRandom(klee::ConstraintSet &cs);
	ConcreteStore getStore(const klee::Assignment &assign);
};

};

#endif
