//===- FunGTDialect.td - FunGT dialect -----------*- tablegen -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef FUNGT_DIALECT
#define FUNGT_DIALECT

include "mlir/IR/OpBase.td"

//===----------------------------------------------------------------------===//
// FunGT dialect definition.
//===----------------------------------------------------------------------===//

def FunGT_Dialect : Dialect {
    let name = "fungt";
    let summary = "A fungt out-of-tree MLIR dialect.";
    let description = [{
        This dialect is an example of an out-of-tree MLIR dialect designed to
        illustrate the basic setup required to develop MLIR-based tools without
        working inside of the LLVM source tree.
    }];
    let cppNamespace = "::mlir::fungt";

    let useDefaultTypePrinterParser = 1;
    let extraClassDeclaration = [{
        void registerTypes();
    }];
}

//===----------------------------------------------------------------------===//
// Base fungt operation definition.
//===----------------------------------------------------------------------===//

class FunGT_Op<string mnemonic, list<Trait> traits = []> :
        Op<FunGT_Dialect, mnemonic, traits>;

#endif // FUNGT_DIALECT
