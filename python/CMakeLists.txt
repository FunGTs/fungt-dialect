include(AddMLIRPython)

# Specifies that all MLIR packages are co-located under the `mlir_fungt`
# top level package (the API has been embedded in a relocatable way).
# TODO: Add an upstream cmake param for this vs having a global here.
add_compile_definitions("MLIR_PYTHON_PACKAGE_PREFIX=mlir_fungt.")


################################################################################
# Sources
################################################################################

declare_mlir_python_sources(FunGTPythonSources)

declare_mlir_dialect_python_bindings(
  ADD_TO_PARENT FunGTPythonSources
  ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mlir_fungt"
  TD_FILE dialects/FunGTOps.td
  SOURCES
    dialects/fungt_pybind11.py
    dialects/fungt_nanobind.py
  DIALECT_NAME fungt)


declare_mlir_python_extension(FunGTPythonSources.Pybind11Extension
  MODULE_NAME _fungtDialectsPybind11
  ADD_TO_PARENT FunGTPythonSources
  SOURCES
    FunGTExtensionPybind11.cpp
  EMBED_CAPI_LINK_LIBS
    FunGTCAPI
  PYTHON_BINDINGS_LIBRARY pybind11
)

declare_mlir_python_extension(FunGTPythonSources.NanobindExtension
  MODULE_NAME _fungtDialectsNanobind
  ADD_TO_PARENT FunGTPythonSources
  SOURCES
    FunGTExtensionNanobind.cpp
  EMBED_CAPI_LINK_LIBS
    FunGTCAPI
  PYTHON_BINDINGS_LIBRARY nanobind
)


################################################################################
# Common CAPI
################################################################################

add_mlir_python_common_capi_library(FunGTPythonCAPI
  INSTALL_COMPONENT FunGTPythonModules
  INSTALL_DESTINATION python_packages/fungt/mlir_fungt/_mlir_libs
  OUTPUT_DIRECTORY "${MLIR_BINARY_DIR}/python_packages/fungt/mlir_fungt/_mlir_libs"
  RELATIVE_INSTALL_ROOT "../../../.."
  DECLARED_SOURCES
    FunGTPythonSources
    # TODO: Remove this in favor of showing fine grained registration once
    # available.
    MLIRPythonExtension.RegisterEverything
    MLIRPythonSources.Core
)

################################################################################
# Instantiation of all Python modules
################################################################################

add_mlir_python_modules(FunGTPythonModules
  ROOT_PREFIX "${MLIR_BINARY_DIR}/python_packages/fungt/mlir_fungt"
  INSTALL_PREFIX "python_packages/fungt/mlir_fungt"
  DECLARED_SOURCES
    FunGTPythonSources
    # TODO: Remove this in favor of showing fine grained registration once
    # available.
    MLIRPythonExtension.RegisterEverything
    MLIRPythonSources
  COMMON_CAPI_LINK_LIBS
    FunGTPythonCAPI
  )
