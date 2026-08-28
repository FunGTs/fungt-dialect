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
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVAttributes.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVEnums.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
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

// ================= HERE STARTS: FunGTShaderLowerToSPIRV =================
#define GEN_PASS_DEF_FUNGTSHADERLOWERTOSPIRV
#include "FunGT/FunGTPasses.h.inc"

namespace {

class ShaderEntryLowering : public OpRewritePattern<ShaderEntryOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(ShaderEntryOp op,
                                 PatternRewriter &rewriter) const override {
    auto moduleOp = op->getParentOfType<ModuleOp>();
    rewriter.setInsertionPointToStart(moduleOp.getBody());

    auto spvModule = rewriter.create<spirv::ModuleOp>(
        op.getLoc(), spirv::AddressingModel::Logical,
        spirv::MemoryModel::GLSL450,
        spirv::VerCapExtAttr::get(
            spirv::Version::V_1_0, {spirv::Capability::Shader}, {},
            rewriter.getContext()));

    rewriter.setInsertionPointToStart(spvModule.getBody());

    auto funcType = rewriter.getFunctionType({}, {});
    auto spvFunc = rewriter.create<spirv::FuncOp>(
        op.getLoc(), op.getSymName(), funcType);
    rewriter.createBlock(&spvFunc.getBody());
    rewriter.setInsertionPointToStart(&spvFunc.getBody().front());

    llvm::DenseMap<Value, Value> valueMap;
    SmallVector<Attribute, 2> interfaceVars;
    int globalCounter = 0;

    for (Operation &bodyOp : op.getBody().front()) {
      if (auto constOp = dyn_cast<ConstVec4Op>(bodyOp)) {
        SmallVector<float, 4> vals = {
            constOp.getXval().convertToFloat(),
            constOp.getYval().convertToFloat(),
            constOp.getZval().convertToFloat(),
            constOp.getWval().convertToFloat()};
        auto vecType = VectorType::get({4}, rewriter.getF32Type());
        auto constant = rewriter.create<spirv::ConstantOp>(
            constOp.getLoc(), vecType,
            DenseElementsAttr::get(vecType, ArrayRef<float>(vals)));
        valueMap[constOp.getResult()] = constant.getResult();
        continue;
      }

      if (auto outOp = dyn_cast<OutputOp>(bodyOp)) {
        auto vecType = outOp.getValue().getType();
        auto ptrType = spirv::PointerType::get(vecType,
                                                spirv::StorageClass::Output);

        std::string globalName =
            ("out_var_" + Twine(globalCounter++)).str();

        {
          OpBuilder::InsertionGuard guard(rewriter);
          rewriter.setInsertionPointToStart(spvModule.getBody());
          auto globalVar = rewriter.create<spirv::GlobalVariableOp>(
              outOp.getLoc(), ptrType, globalName,
              /*initializer=*/nullptr);
          globalVar->setAttr("location", outOp.getLocationAttr());
        }

        auto addr = rewriter.create<spirv::AddressOfOp>(
            outOp.getLoc(), ptrType,
            SymbolRefAttr::get(rewriter.getContext(), globalName));
        rewriter.create<spirv::StoreOp>(
            outOp.getLoc(), addr.getResult(), valueMap[outOp.getValue()]);

        interfaceVars.push_back(
            SymbolRefAttr::get(rewriter.getContext(), globalName));
        continue;
      }
      if (auto inOp = dyn_cast<InputOp>(bodyOp)) {
            auto vecType = inOp.getResult().getType();
            auto ptrType = spirv::PointerType::get(vecType,
                                                    spirv::StorageClass::Input);
            std::string globalName =
                ("in_var_" + Twine(globalCounter++)).str();

            {
                OpBuilder::InsertionGuard guard(rewriter);
                rewriter.setInsertionPointToStart(spvModule.getBody());
                auto globalVar = rewriter.create<spirv::GlobalVariableOp>(
                    inOp.getLoc(), ptrType, globalName,
                    /*initializer=*/nullptr);
                globalVar->setAttr("location", inOp.getLocationAttr());
            }

            auto addr = rewriter.create<spirv::AddressOfOp>(
                inOp.getLoc(), ptrType,
                SymbolRefAttr::get(rewriter.getContext(), globalName));
            auto loaded = rewriter.create<spirv::LoadOp>(inOp.getLoc(), addr.getResult());

            valueMap[inOp.getResult()] = loaded.getResult();
            interfaceVars.push_back(
                SymbolRefAttr::get(rewriter.getContext(), globalName));
            continue;
       }
       if (isa<ShaderEndOp>(bodyOp)) {
            rewriter.create<spirv::ReturnOp>(bodyOp.getLoc());
            continue;
        }
    }

    StringRef execModelStr = op.getExecutionModel();
    spirv::ExecutionModel execModel =
        execModelStr == "Fragment" ? spirv::ExecutionModel::Fragment
                                    : spirv::ExecutionModel::Vertex;

    rewriter.setInsertionPointToEnd(spvModule.getBody());
    rewriter.create<spirv::EntryPointOp>(
        op.getLoc(), execModel, spvFunc,
        interfaceVars);

    if (execModel == spirv::ExecutionModel::Fragment) {
      rewriter.create<spirv::ExecutionModeOp>(
          op.getLoc(), spvFunc,
          spirv::ExecutionMode::OriginUpperLeft, ArrayRef<int32_t>{});
    }

    rewriter.eraseOp(op);
    return success();
  }
};

class FunGTShaderLowerToSPIRV
    : public impl::FunGTShaderLowerToSPIRVBase<FunGTShaderLowerToSPIRV> {
public:
  using impl::FunGTShaderLowerToSPIRVBase<FunGTShaderLowerToSPIRV>::FunGTShaderLowerToSPIRVBase;
  void runOnOperation() final {
    RewritePatternSet patterns(&getContext());
    patterns.add<ShaderEntryLowering>(&getContext());
    FrozenRewritePatternSet patternSet(std::move(patterns));
    if (failed(applyPatternsGreedily(getOperation(), patternSet)))
      signalPassFailure();
  }
};

} // namespace
// ================= HERE ENDS: FunGTShaderLowerToSPIRV =================


} // namespace mlir::fungt
