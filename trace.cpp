#include <clover/clover.h>

using namespace clover;

Trace::Trace(Solver &_solver)
		: solver(_solver), csm(cs)
{
	return;
}

void
Trace::add(klee::ref<klee::Expr> expr)
{
	csm.addConstraint(expr);
	return;
}
