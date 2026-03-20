add_mlir_dialect(FunGTOps fungt)
add_mlir_doc(FunGTDialect FunGTDialect FunGT/ -gen-dialect-doc)
add_mlir_doc(FunGTOps FunGTOps FunGT/ -gen-op-doc)

set(LLVM_TARGET_DEFINITIONS FunGTPasses.td)
mlir_tablegen(FunGTPasses.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIRFunGTPassesIncGen)
