add_mlir_dialect_library(MLIRFunGT
        FunGTTypes.cpp
        FunGTDialect.cpp
        FunGTOps.cpp
        FunGTPasses.cpp

        ADDITIONAL_HEADER_DIRS
        ${PROJECT_SOURCE_DIR}/include/FunGT

        DEPENDS
        MLIRFunGTOpsIncGen
        MLIRFunGTPassesIncGen

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRInferTypeOpInterface
        MLIRFuncDialect
        MLIRArithDialect
        MLIRMathDialect
        )
