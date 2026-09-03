#include "FunGT/FunGTIRParser.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

int main(int argc, char **argv) {
    if (argc < 2) {
        llvm::errs() << "Usage: fungt-parse [--shader] <input.fgt>\n";
        return 1;
    }

    bool shaderMode = false;
    std::string inputFile;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--shader") {
            shaderMode = true;
        } else {
            inputFile = arg;
        }
    }

    if (inputFile.empty()) {
        llvm::errs() << "No input file specified\n";
        return 1;
    }

    auto fileOrErr = llvm::MemoryBuffer::getFile(inputFile);
    if (!fileOrErr) {
        llvm::errs() << "Could not open file: " << inputFile << "\n";
        return 1;
    }

    mlir::MLIRContext ctx;
    mlir::registerAllDialects(ctx);

    mlir::OwningOpRef<mlir::ModuleOp> moduleOp;
    if (shaderMode) {
        moduleOp = mlir::fungt::parseShaderIR(ctx, (*fileOrErr)->getBuffer());
    } else {
        moduleOp = mlir::fungt::parseFunGTIR(ctx, (*fileOrErr)->getBuffer());
    }

    if (!moduleOp) {
        llvm::errs() << "Parse failed\n";
        return 1;
    }

    moduleOp->print(llvm::outs());
    llvm::outs() << "\n";
    return 0;
}