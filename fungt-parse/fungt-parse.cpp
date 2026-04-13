#include "FunGT/FunGTIRParser.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllDialects.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        llvm::errs() << "Usage: fungt-parse <input.fgt>\n";
        return 1;
    }

    auto fileOrErr = llvm::MemoryBuffer::getFile(argv[1]);
    if (!fileOrErr) {
        llvm::errs() << "Could not open file: " << argv[1] << "\n";
        return 1;
    }

    mlir::MLIRContext ctx;
    mlir::registerAllDialects(ctx);

    auto moduleOp = mlir::fungt::parseFunGTIR(ctx, (*fileOrErr)->getBuffer());
    if (!moduleOp) {
        llvm::errs() << "Parse failed\n";
        return 1;
    }

    moduleOp->print(llvm::outs());
    llvm::outs() << "\n";
    return 0;
}
