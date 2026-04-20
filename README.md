# fungt-dialect

MLIR dialect for FunGT physics kernel compilation. Takes FunGT IR as input,
lowers through MLIR to SPIR-V for runtime loading via SYCL.

## Overview

fungt-dialect defines a restricted domain-specific IR for particle physics
kernels. The restricted vocabulary makes it safe for LLM-generated kernels:
the language cannot express GPU memory bugs, out-of-bounds access, or
driver-hanging constructs.

## Ops

**fungt.update** — Per-particle update kernel container. Takes particle state
(position, velocity, mass, dt, age) as input, holds a region body with
arithmetic ops, produces new particle state via fungt.yield.

**fungt.yield** — Terminates the update body. Returns 7 f32 values:
new position (3), new velocity (3), new age (1).

**fungt.distance** — Euclidean distance between two 3D points. Returns f32.

**fungt.select** — Conditional value selection. Takes an i1 condition and two
f32 values, returns the first if true, second if false.

**fungt.scalar_mul** — Multiply two f32 scalars.

The update body also supports standard MLIR ops from the arith and math
dialects: arith.addf, arith.subf, arith.mulf, arith.divf, arith.cmpf,
math.sqrt, math.sin, math.cos.

## Tools

**fungt-opt** — Optimizer driver. Runs lowering passes on MLIR input.

**fungt-parse** — FunGT IR parser. Reads user-friendly .fgt text files and
emits MLIR with fungt ops.

**fungt-translate** — SPIR-V serializer. Converts SPIR-V dialect MLIR to
binary .spv files.

## Pipeline

```
User writes .fgt file (or LLM generates it)
        |
   fungt-parse rain.fgt > rain.mlir
        |
   fungt-opt --fungt-lower-to-arith rain.mlir > rain_lowered.mlir
        |
   fungt-opt --convert-gpu-to-spirv --spirv-lower-abi-attrs --spirv-update-vce
        |
   fungt-translate --fungt-to-spirv -o rain.spv
        |
   SYCL loads rain.spv via kernel_compiler extension
```

## FunGT IR Syntax

The user or LLM writes in a simple text format:

```
update rain:
    gravity = -9.8
    new_vz = vel_z + gravity * dt
    new_pz = pos_z + new_vz * dt
    new_age = age + dt
    yield pos_x, pos_y, new_pz, vel_x, vel_y, new_vz, new_age
```

Built-in variables: pos_x, pos_y, pos_z, vel_x, vel_y, vel_z, mass, dt, age

Built-in functions: sqrt(), sin(), cos(), distance(), select()

Operators: +, -, *, /, <, >

## Lowering Passes

**fungt-lower-to-arith** — Lowers all fungt ops to arith and math dialect ops.
Pattern replacements:

    fungt.scalar_mul  -> arith.mulf
    fungt.select      -> arith.select
    fungt.distance    -> arith.subf, arith.mulf, arith.addf, math.sqrt
    fungt.update      -> region inlining (ops moved to parent scope)
    fungt.yield       -> erased (handled by update lowering)

## Building

Requires an LLVM/MLIR build. Point CMake at your MLIR install:

```bash
mkdir build && cd build
cmake -G Ninja .. \
  -DMLIR_DIR=/path/to/llvm-project/build/lib/cmake/mlir \
  -DLLVM_EXTERNAL_LIT=/path/to/llvm-project/build/bin/llvm-lit
ninja
```

## Testing

Parse and lower a FunGT IR file:

```bash
./build/bin/fungt-parse fgt_test/rain.fgt | ./build/bin/fungt-opt --fungt-lower-to-arith
```

Verify ops parse correctly:

```bash
./build/bin/fungt-opt fgt_test/rain.mlir
```

Generate SPIR-V binary:

```bash
./build/bin/fungt-opt --convert-gpu-to-spirv --spirv-lower-abi-attrs --spirv-update-vce fgt_test/rain_gpu.mlir > fgt_test/rain_spirv.mlir
./build/bin/fungt-translate --fungt-to-spirv fgt_test/rain_spirv.mlir -o fgt_test/rain.spv
```

## Part of FunGT

https://github.com/FunGTs/FunGT

## License

Apache 2.0 with LLVM Exceptions. See LICENSE for details.