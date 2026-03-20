get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)
set(LIBS
        ${dialect_libs}
        ${conversion_libs}
        MLIRArithDialect
        MLIROptLib
        MLIRFunGT
        )
add_llvm_executable(fungt-opt fungt-opt.cpp)

llvm_update_compile_flags(fungt-opt)
target_link_libraries(fungt-opt PRIVATE ${LIBS})

mlir_check_all_link_libraries(fungt-opt)
