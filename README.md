# fungt-dialect

MLIR dialect for FunGT physics kernel compilation. Takes FunGT IR as input,
lowers through MLIR to SPIR-V for runtime loading via SYCL.

## Overview

fungt-dialect defines a restricted domain-specific IR for particle physics
force laws. The restricted vocabulary makes it safe for LLM-generated kernels
— the language cannot express GPU memory bugs, out-of-bounds access, or
driver-hanging constructs.

## Ops

**fungt.distance** — Euclidean distance between two 3D points. Returns f32.

**fungt.guard** — Early exit if condition is false. Force contribution becomes
zero. The only conditional in the language.

**fungt.scalar_mul** — Multiply two f32 scalars.

## Pipeline
```
FunGT IR text → fungt-opt → arith/math → gpu dialect → spirv dialect → .spv binary
```

The SPIR-V binary is loaded at runtime by FunGT via the SYCL kernel_compiler
extension.

## Building

Requires an LLVM/MLIR build. Point CMake at your MLIR install:
```bash
mkdir build && cd build
cmake -G Ninja .. \
  -DMLIR_DIR=/path/to/llvm-project/build/lib/cmake/mlir \
  -DLLVM_EXTERNAL_LIT=/path/to/llvm-project/build/bin/llvm-lit
ninja
```

## Part of FunGT

https://github.com/FunGTs/FunGT

## License

Apache 2.0 with LLVM Exceptions. See LICENSE for details.
