add_llvm_library(FunGTPlugin
        # BUILDTREE_ONLY is only for testing purposes
        MODULE BUILDTREE_ONLY
        fungt-plugin.cpp

        DEPENDS
        MLIRFunGT
        PLUGIN_TOOL
        mlir-opt

        LINK_LIBS
        MLIRFunGT
        )
target_include_directories(
        FunGTPlugin
        PRIVATE
        "${FUNGT_BINARY_DIR}/include"
)
