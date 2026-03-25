//===- FunGTPsss.td - FunGT dialect passes -------*- tablegen -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef FUNGT_PASS
#define FUNGT_PASS

include "mlir/Pass/PassBase.td"

def FunGTSwitchBarFoo: Pass<"fungt-switch-bar-foo", "::mlir::ModuleOp"> {
  let summary = "Switches the name of a FuncOp named `bar` to `foo` and folds.";
  let description = [{
    Switches the name of a FuncOp named `bar` to `foo` and folds.
    ```
    func.func @bar() {
      return
    }
    // Gets transformed to:
    func.func @foo() {
      return
    }
    ```
  }];
}
def FunGTLowerToArith: Pass<"fungt-lower-to-arith", "::mlir::ModuleOp"> {
  let summary = "Lower FunGT dialect ops to arith and math dialect ops.";
  let description = [{
    Lowers fungt.scalar_mul to arith.mulf, fungt.distance to a sequence
    of arith and math ops, and fungt.select to arith.select.
  }];
  let dependentDialects = [
    "mlir::arith::ArithDialect",
    "mlir::math::MathDialect"
  ];
}

#endif // FUNGT_PASS
