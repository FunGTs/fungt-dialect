#include "mlir/InitAllDialects.h"
#include "mlir/InitAllTranslations.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"
#include "mlir/Target/SPIRV/Serialization.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/Operation.h"
#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "llvm/Support/raw_ostream.h"

int main(int argc, char **argv) {
  mlir::registerAllTranslations();

  mlir::TranslateFromMLIRRegistration serializeSpirv(
      "fungt-to-spirv", "Serialize spirv.module to binary",
      [](mlir::Operation *op, llvm::raw_ostream &output) {
        for (auto &region : op->getRegions())
          for (auto &block : region)
            for (auto &inner : block)
              if (auto spirvMod = llvm::dyn_cast<mlir::spirv::ModuleOp>(inner)) {
                llvm::SmallVector<uint32_t, 0> binary;
                if (mlir::spirv::serialize(spirvMod, binary).failed())
                  return mlir::failure();
                output.write(reinterpret_cast<const char *>(binary.data()),
                             binary.size() * sizeof(uint32_t));
                return mlir::success();
              }
        return mlir::failure();
      },
      [](mlir::DialectRegistry &registry) {
        mlir::registerAllDialects(registry);
      });

  return failed(
      mlir::mlirTranslateMain(argc, argv, "FunGT Translation Tool"));
}