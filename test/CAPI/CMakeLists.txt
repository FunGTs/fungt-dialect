# Here we create a single aggregate shared library with the parts of the CAPI
# that we want to bundle together. Then we link a simple C executable
# against it to demonstrate that it does have the fully self contained
# core MLIR library and our own fungt dialect.
add_mlir_aggregate(FunGTCAPITestLib
  SHARED
  EMBED_LIBS
  MLIRCAPIIR
  # TODO: Remove this in favor of showing fine grained dialect registration
  # (once available).
  MLIRCAPIRegisterEverything
  FunGTCAPI
)

add_llvm_executable(fungt-capi-test
  fungt-capi-test.c
)
llvm_update_compile_flags(fungt-capi-test)
target_link_libraries(fungt-capi-test
  PRIVATE FunGTCAPITestLib)
