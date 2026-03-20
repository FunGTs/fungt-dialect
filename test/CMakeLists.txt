llvm_canonicalize_cmake_booleans(
        MLIR_ENABLE_BINDINGS_PYTHON
)

configure_lit_site_cfg(
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
        ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
        MAIN_CONFIG
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(FUNGT_TEST_DEPENDS
        FileCheck count not
        fungt-capi-test
        fungt-opt
        fungt-translate
        FunGTPlugin
        )
if(MLIR_ENABLE_BINDINGS_PYTHON)
  list(APPEND FUNGT_TEST_DEPENDS FunGTPythonModules)
endif()

add_lit_testsuite(check-fungt "Running the fungt regression tests"
        ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${FUNGT_TEST_DEPENDS}
        )
set_target_properties(check-fungt PROPERTIES FOLDER "Tests")

add_lit_testsuites(FUNGT ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${FUNGT_TEST_DEPENDS})

add_subdirectory(CAPI)
