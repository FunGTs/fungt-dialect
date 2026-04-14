get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
set(LIBS
        ${dialect_libs}
        MLIRArithDialect
        MLIRMathDialect
        MLIRFuncDialect
        MLIRIR
        MLIRParser
        MLIRFunGT
        )
add_llvm_executable(fungt-parse fungt-parse.cpp)
llvm_update_compile_flags(fungt-parse)
target_link_libraries(fungt-parse PRIVATE ${LIBS})
mlir_check_all_link_libraries(fungt-parse)
