//===- FunGTOps.td - FunGT dialect ops -----------*- tablegen -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef FUNGT_OPS
#define FUNGT_OPS

include "FunGT/FunGTTypes.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

def FunGT_DistanceOp : FunGT_Op<"distance", [Pure]> {
    let summary = "Euclidean distance between two 3D points.";
    let arguments = (ins F32:$x0, F32:$y0, F32:$z0,
                         F32:$x1, F32:$y1, F32:$z1);
    let results = (outs F32:$result);
    let assemblyFormat = [{
        $x0 `,` $y0 `,` $z0 `,` $x1 `,` $y1 `,` $z1 attr-dict `:` type($result)
    }];
}

def FunGT_GuardOp : FunGT_Op<"guard", []> {
    let summary = "Early exit if condition is false.";
    let arguments = (ins I1:$condition);
    let assemblyFormat = [{
        $condition attr-dict
    }];
}

def FunGT_ScalarMulOp : FunGT_Op<"scalar_mul", [Pure]> {
    let summary = "Multiply two f32 scalars.";
    let arguments = (ins F32:$lhs, F32:$rhs);
    let results = (outs F32:$result);
    let assemblyFormat = [{
        $lhs `,` $rhs attr-dict `:` type($result)
    }];
}

#endif // FUNGT_OPS