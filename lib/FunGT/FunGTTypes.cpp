//===- FunGTTypes.cpp - FunGT dialect types -----------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FunGT/FunGTTypes.h"

#include "FunGT/FunGTDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "llvm/ADT/TypeSwitch.h"

using namespace mlir::fungt;

#define GET_TYPEDEF_CLASSES
#include "FunGT/FunGTOpsTypes.cpp.inc"

void FunGTDialect::registerTypes() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "FunGT/FunGTOpsTypes.cpp.inc"
      >();
}
