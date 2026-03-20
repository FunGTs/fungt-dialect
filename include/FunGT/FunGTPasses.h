//===- FunGTPasses.h - FunGT passes  ------------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef FUNGT_FUNGTPASSES_H
#define FUNGT_FUNGTPASSES_H

#include "FunGT/FunGTDialect.h"
#include "FunGT/FunGTOps.h"
#include "mlir/Pass/Pass.h"
#include <memory>

namespace mlir {
namespace fungt {
#define GEN_PASS_DECL
#include "FunGT/FunGTPasses.h.inc"

#define GEN_PASS_REGISTRATION
#include "FunGT/FunGTPasses.h.inc"
} // namespace fungt
} // namespace mlir

#endif
