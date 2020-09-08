#ifndef CLOVER_H
#define CLOVER_H

#include <stdbool.h>
#include <stdint.h>

#include <klee/Expr/ArrayCache.h>
#include <klee/Expr/Assignment.h>
#include <klee/Expr/Expr.h>
#include <klee/Expr/ExprBuilder.h>
#include <klee/Solver/Solver.h>

#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <variant>

namespace clover {

typedef std::variant<uint8_t, uint32_t> IntValue;

class BitVector {
private:
	klee::ref<klee::Expr> expr;

	BitVector(const klee::ref<klee::Expr> &expr);
	BitVector(IntValue value);
	BitVector(const klee::Array *array);

	std::shared_ptr<BitVector> negate(void);

	friend class ConcolicValue;
	friend class Solver;
	friend class Trace;
};

class ConcolicValue {
public:
	std::shared_ptr<BitVector> concrete;
	std::optional<std::shared_ptr<BitVector>> symbolic;

	unsigned getWidth(void);

	std::shared_ptr<ConcolicValue> eq(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> ne(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> lshl(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> lshr(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> ashr(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> add(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> sub(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> slt(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> sge(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> ult(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> uge(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> band(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> bor(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> bxor(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> concat(std::shared_ptr<ConcolicValue> other);
	std::shared_ptr<ConcolicValue> bnot(void);
	std::shared_ptr<ConcolicValue> extract(unsigned offset, klee::Expr::Width width);
	std::shared_ptr<ConcolicValue> sext(klee::Expr::Width width);
	std::shared_ptr<ConcolicValue> zext(klee::Expr::Width width);

private:
	klee::ExprBuilder *builder = NULL;

	ConcolicValue(klee::ExprBuilder *_builder,
	              std::shared_ptr<BitVector> _concrete,
	              std::optional<std::shared_ptr<BitVector>> _symbolic = std::nullopt);

	/* The solver acts as a factory for ConcolicValue */
	friend class Solver;
};

class Solver {
private:
	klee::Solver *solver;
	klee::ArrayCache array_cache;
	klee::ExprBuilder *builder = NULL;

public:
	Solver(klee::Solver *_solver = NULL);
	~Solver(void);

	std::optional<klee::Assignment> getAssignment(const klee::Query &query);

	bool eval(const klee::Query &query);
	bool eval(std::shared_ptr<BitVector> bv);

	std::shared_ptr<ConcolicValue> BVC(std::optional<std::string> name, IntValue value);

	template <typename T>
	T evalValue(const klee::Query &query)
	{
		klee::ref<klee::ConstantExpr> r;

		if (!solver->getValue(query, r))
			throw std::runtime_error("getValue() failed for solver");

		return (T)r->getZExtValue(sizeof(T) * 8);
	}

	template <typename T>
	T evalValue(std::shared_ptr<BitVector> bv)
	{
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
	void reset(void);

	std::shared_ptr<ConcolicValue> load(Addr addr, unsigned bytesize);
	std::shared_ptr<ConcolicValue> load(std::shared_ptr<ConcolicValue> addr, unsigned bytesize);

	void store(Addr addr, std::shared_ptr<ConcolicValue> value, unsigned bytesize);
	void store(std::shared_ptr<ConcolicValue> addr, std::shared_ptr<ConcolicValue> value, unsigned bytesize);
};

typedef std::map<std::string, IntValue> ConcreteStore;

class Trace {
private:
	class Branch {
	public:
		typedef std::pair<std::shared_ptr<BitVector>, bool> PathElement;
		typedef std::vector<PathElement> Path;

		std::shared_ptr<BitVector> bv;
		bool wasNegated; /* Don't negate nodes twice (could be unsat) */

		std::shared_ptr<Branch> true_branch;
		std::shared_ptr<Branch> false_branch;

		Branch(void);
		bool isPlaceholder(void);

		/* Returns seemingly random path to a node in the tree
		 * for which either the false or the true branch wasn't
		 * attempted to be taken yet. If no such node exists,
		 * false is returned. */
		bool getRandomPath(Path &path);
	};

	Solver &solver;

	std::shared_ptr<Branch> pathCondsRoot;
	std::shared_ptr<Branch> pathCondsCurrent;

	klee::Query getQuery(klee::ConstraintSet &cs, Branch::Path &path);

public:
	Trace(Solver &_solver);
	void reset(void);

	void add(bool condition, std::shared_ptr<BitVector> bv);

	std::optional<klee::Assignment> findNewPath(klee::ConstraintSet &cs);
	ConcreteStore getStore(const klee::Assignment &assign);
};

class ExecutionContext {
private:
	std::unordered_map<std::string, IntValue> names;

	Solver &solver;

	template <typename T>
	IntValue findRemoveOrRandom(std::string name)
	{
		IntValue concrete;

		auto iter = names.find(name);
		if (iter != names.end()) {
			concrete = (*iter).second;
			assert(std::get_if<T>(&concrete) != nullptr);
			names.erase(iter);
		} else {
			concrete = (T)rand();
		}

		return concrete;
	}

public:
	typedef uint32_t Address;

	ExecutionContext(Solver &_solver);
	bool hasNewPath(Trace &trace);

	template <typename T>
	std::shared_ptr<ConcolicValue> getSymbolic(std::string name)
	{
		IntValue concrete = findRemoveOrRandom<T>(name);
		return solver.BVC(name, concrete);
	}

	std::shared_ptr<ConcolicValue> getSymbolic(size_t reg);
	std::shared_ptr<ConcolicValue> getSymbolic(Address addr, size_t len);
};

}; // namespace clover

#endif
