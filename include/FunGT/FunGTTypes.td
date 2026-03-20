//===- FunGTTypes.td - FunGT dialect types -------*- tablegen -*-===//
//
// This file is licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef FUNGT_TYPES
#define FUNGT_TYPES

include "mlir/IR/AttrTypeBase.td"
include "FunGT/FunGTDialect.td"

//===----------------------------------------------------------------------===//
// FunGT type definitions
//===----------------------------------------------------------------------===//

class FunGT_Type<string name, string typeMnemonic, list<Trait> traits = []>
    : TypeDef<FunGT_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def FunGT_CustomType : FunGT_Type<"Custom", "custom"> {
    let summary = "FunGT custom type";
    let description = "Custom type in fungt dialect";
    let parameters = (ins StringRefParameter<"the custom value">:$value);
    let assemblyFormat = "`<` $value `>`";
}

#endif // FUNGT_TYPES
