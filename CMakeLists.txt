set(LLVM_LIBS ${LLVM_AVAILABLE_LIBS})

add_library(clover solver.cpp bitvector.cpp)
target_include_directories(clover PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(clover PUBLIC
	${KLEE_LIBS} ${Z3_LIBS} ${STP_LIBS} ${LLVM_LIBS} z)
