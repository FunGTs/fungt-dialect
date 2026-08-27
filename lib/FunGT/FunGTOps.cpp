//===- FunGTOps.cpp - FunGT dialect ops ---------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "mlir/IR/OpImplementation.h"
#include "FunGT/FunGTOps.h"
#include "FunGT/FunGTDialect.h"

#define GET_OP_CLASSES
#include "FunGT/FunGTOps.cpp.inc"
namespace mlir::fungt {

LogicalResult ResourceBindingOp::verify() {
  llvm::StringRef storageClass = getStorageClass();
  if (storageClass != "UniformConstant" &&
      storageClass != "Uniform" &&
      storageClass != "StorageBuffer") {
    return emitOpError("storage_class must be one of UniformConstant, "
                        "Uniform, or StorageBuffer, got '")
           << storageClass << "'";
  }
  return success();
}
LogicalResult ShaderEntryOp::verify() {
  llvm::StringRef executionModel = getExecutionModel();
  if (executionModel != "Vertex" && executionModel != "Fragment") {
    return emitOpError("execution_model must be 'Vertex' or 'Fragment', got '")
           << executionModel << "'";
  }
  return success();
}

} // namespace mlir::fungt