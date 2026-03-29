//===- FunGTPasses.cpp - FunGT passes -----------------*- C++ -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Rewrite/FrozenRewritePatternSet.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "FunGT/FunGTPasses.h"

namespace mlir::fungt {
#define GEN_PASS_DEF_FUNGTSWITCHBARFOO
#include "FunGT/FunGTPasses.h.inc"

namespace {
class FunGTSwitchBarFooRewriter : public OpRewritePattern<func::FuncOp> {
public:
  using OpRewritePattern<func::FuncOp>::OpRewritePattern;
  LogicalResult matchAndRewrite(func::FuncOp op,
                                PatternRewriter &rewriter) const final {
    if (op.getSymName() == "bar") {
      rewriter.modifyOpInPlace(op, [&op]() { op.setSymName("foo"); });
      return success();
    }
    return failure();
  }
};

class FunGTSwitchBarFoo
    : public impl::FunGTSwitchBarFooBase<FunGTSwitchBarFoo> {
public:
  using impl::FunGTSwitchBarFooBase<
      FunGTSwitchBarFoo>::FunGTSwitchBarFooBase;
  void runOnOperation() final {
    RewritePatternSet patterns(&getContext());
    patterns.add<FunGTSwitchBarFooRewriter>(&getContext());
    FrozenRewritePatternSet patternSet(std::move(patterns));
    if (failed(applyPatternsGreedily(getOperation(), patternSet)))
      signalPassFailure();
  }
};
} // namespace
#define GEN_PASS_DEF_FUNGTLOWERTOARITH
#include "FunGT/FunGTPasses.h.inc"

namespace {

class ScalarMulLowering : public OpRewritePattern<ScalarMulOp> {
public:
    using OpRewritePattern::OpRewritePattern;
    LogicalResult matchAndRewrite(ScalarMulOp op,
                                  PatternRewriter &rewriter) const override {
        rewriter.replaceOpWithNewOp<arith::MulFOp>(op, op.getLhs(), op.getRhs());
        return success();
    }
};
class SelectLowering : public OpRewritePattern<SelectOp> {
public:
    using OpRewritePattern::OpRewritePattern;
    LogicalResult matchAndRewrite(SelectOp op,
                                  PatternRewriter &rewriter) const override {
        rewriter.replaceOpWithNewOp<arith::SelectOp>(
            op, op.getCondition(), op.getTrueVal(), op.getFalseVal());
        return success();
    }
};
class FunGTLowerToArith
    : public impl::FunGTLowerToArithBase<FunGTLowerToArith> {
public:
    using impl::FunGTLowerToArithBase<FunGTLowerToArith>::FunGTLowerToArithBase;
    void runOnOperation() final {
        RewritePatternSet patterns(&getContext());
        patterns.add<ScalarMulLowering>(&getContext());
        patterns.add<SelectLowering>(&getContext());
        FrozenRewritePatternSet patternSet(std::move(patterns));
        if (failed(applyPatternsGreedily(getOperation(), patternSet)))
            signalPassFailure();
    }
};

} // namespace
} // namespace mlir::fungt
