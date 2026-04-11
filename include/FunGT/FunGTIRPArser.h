#if !defined(_FUNGT_IR_PARSER_H_)
#define _FUNGT_IR_PARSER_H_

#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/OwningOpRef.h"
#include "llvm/ADT/StringRef.h"

namespace mlir {
namespace fungt {

mlir::OwningOpRef<mlir::ModuleOp>
parseFunGTIR(mlir::MLIRContext &ctx, llvm::StringRef source);

} // namespace fungt
} // namespace mlir

#endif // _FUNGT_IR_PARSER_H_