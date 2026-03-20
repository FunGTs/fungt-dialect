//===- FunGTDialect.cpp - FunGT dialect ---------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "FunGT/FunGTDialect.h"
#include "FunGT/FunGTOps.h"
#include "FunGT/FunGTTypes.h"

using namespace mlir;
using namespace mlir::fungt;

#include "FunGT/FunGTOpsDialect.cpp.inc"

//===----------------------------------------------------------------------===//
// FunGT dialect.
//===----------------------------------------------------------------------===//

void FunGTDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "FunGT/FunGTOps.cpp.inc"
      >();
  registerTypes();
}
