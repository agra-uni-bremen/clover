# clover

A library for concolic execution of RV32 instruction set simulators.

This library is based on KLEE but currently only uses KLEE as an
abstraction over Z3/STP. A long term goal would be to include only the
required KLEE code in this repository to prevent a dependency on LLVM
(e.g. vendor a modified KLEE version in this repository as done by S²E).

## Development

A pre-commit git hook for checking if files are properly formatted is
provided in `.githooks`. It can be activated using:

	$ git config --local core.hooksPath .githooks
