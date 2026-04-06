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
class DistanceLowering : public OpRewritePattern<DistanceOp>{

public: 
    using OpRewritePattern::OpRewritePattern;

     LogicalResult matchAndRewrite(DistanceOp op,
                                   PatternRewriter &rewriter) const override {
        auto dx = rewriter.create<arith::SubFOp>(op.getLoc(), op.getX0(), op.getX1());   
        auto dy = rewriter.create<arith::SubFOp>(op.getLoc(), op.getY0(), op.getY1());        
        auto dz = rewriter.create<arith::SubFOp>(op.getLoc(), op.getZ0(), op.getZ1());
        
        auto dx2 = rewriter.create<arith::MulFOp>(op.getLoc(), dx.getResult(), dx.getResult());
        auto dy2 = rewriter.create<arith::MulFOp>(op.getLoc(),dy.getResult(), dy.getResult());
        auto dz2 = rewriter.create<arith::MulFOp>(op.getLoc(),dz.getResult(), dz.getResult());

        auto sum1 = rewriter.create<arith::AddFOp>(op.getLoc(),dx2.getResult(),dy2.getResult());
        auto sum2 = rewriter.create<arith::AddFOp>(op.getLoc(),sum1.getResult(),dz2.getResult());

        rewriter.replaceOpWithNewOp<math::SqrtOp>(op, sum2);
        return success();

    }

};
class UpdateLowering : public OpRewritePattern<Update> {
public:
    using OpRewritePattern::OpRewritePattern;
    LogicalResult matchAndRewrite(Update op,
                                  PatternRewriter &rewriter) const override {
        Block &body = op.getBody().front();

        // Map block arguments to the update op's input arguments
        for (auto [blockArg, operand] :
             llvm::zip(body.getArguments(), op.getOperands())) {
            blockArg.replaceAllUsesWith(operand);
        }

        // Find the yield terminator
        auto yieldOp = cast<YieldOp>(body.getTerminator());

        // Replace update results with yield arguments
        for (auto [result, yieldArg] :
             llvm::zip(op.getResults(), yieldOp.getOperands())) {
            result.replaceAllUsesWith(yieldArg);
        }

        // Move all ops from the region body to before the update op
        rewriter.setInsertionPoint(op);
        for (auto &bodyOp : llvm::make_early_inc_range(body.without_terminator())) {
            bodyOp.moveBefore(op);
        }

        // Erase yield and update
        rewriter.eraseOp(yieldOp);
        rewriter.eraseOp(op);

        return success();
    }
};
//This is the pass
class FunGTLowerToArith
    : public impl::FunGTLowerToArithBase<FunGTLowerToArith> {
public:
    using impl::FunGTLowerToArithBase<FunGTLowerToArith>::FunGTLowerToArithBase;
    void runOnOperation() final {
        RewritePatternSet patterns(&getContext());
        patterns.add<ScalarMulLowering>(&getContext());
        patterns.add<SelectLowering>(&getContext());
        patterns.add<DistanceLowering>(&getContext());
        patterns.add<UpdateLowering>(&getContext());
        FrozenRewritePatternSet patternSet(std::move(patterns));
        if (failed(applyPatternsGreedily(getOperation(), patternSet)))
            signalPassFailure();
    }
};

} // namespace
} // namespace mlir::fungt
