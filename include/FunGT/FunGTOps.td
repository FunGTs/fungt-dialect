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
include "mlir/IR/OpAsmInterface.td"

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

def FunGT_ScalarMulOp : FunGT_Op<"scalar_mul", [Pure]> { //Pure means NO SIDE EFFECTS
    let summary = "Multiply two f32 scalars.";
    let arguments = (ins F32:$lhs, F32:$rhs);
    let results = (outs F32:$result);
    let assemblyFormat = [{
        $lhs `,` $rhs attr-dict `:` type($result)
    }];
}
def FunGT_Update : FunGT_Op<"update", []>{


    let summary = "Updating particles.";
    let arguments = (ins F32:$pos_x, F32:$pos_y, F32:$pos_z,
                     F32:$vel_x, F32:$vel_y, F32:$vel_z,
                     F32:$mass, F32:$dt, F32:$age);

    let regions = (region SizedRegion<1>:$body);

    let results = (outs F32:$new_pos_x, F32:$new_pos_y, F32:$new_pos_z,
                    F32:$new_vel_x, F32:$new_vel_y, F32:$new_vel_z,
                    F32:$new_age);
    let assemblyFormat = [{
        `(` $pos_x `,` $pos_y `,` $pos_z `,`
            $vel_x `,` $vel_y `,` $vel_z `,`
            $mass `,` $dt `,` $age `)`
        $body attr-dict `:`
        `(` type($pos_x) `,` type($pos_y) `,` type($pos_z) `,`
            type($vel_x) `,` type($vel_y) `,` type($vel_z) `,`
            type($mass) `,` type($dt) `,` type($age) `)`
        `->` `(` type($new_pos_x) `,` type($new_pos_y) `,` type($new_pos_z) `,`
                type($new_vel_x) `,` type($new_vel_y) `,` type($new_vel_z) `,`
                type($new_age) `)`
    }];
}
def FunGT_YieldOp : FunGT_Op<"yield", [Terminator]> {
    let summary = "Yield results from update body.";
    let arguments = (ins F32:$out_pos_x, F32:$out_pos_y, F32:$out_pos_z,
                         F32:$out_vel_x, F32:$out_vel_y, F32:$out_vel_z,
                         F32:$out_age);
    let assemblyFormat = [{
    $out_pos_x `,` $out_pos_y `,` $out_pos_z `,`
    $out_vel_x `,` $out_vel_y `,` $out_vel_z `,`
    $out_age attr-dict `:` type($out_pos_x) `,` type($out_pos_y) `,` type($out_pos_z) `,`
    type($out_vel_x) `,` type($out_vel_y) `,` type($out_vel_z) `,` type($out_age)
    }];
}

def FunGT_SelectOp : FunGT_Op<"select", [Pure]> {
    let summary = "Select between two f32 values based on condition.";
    let arguments = (ins I1:$condition, F32:$true_val, F32:$false_val);
    let results = (outs F32:$result);
    let assemblyFormat = [{
    $condition `,` $true_val `,` $false_val attr-dict `:` type($result)
    }];
}
#endif // FUNGT_OPS