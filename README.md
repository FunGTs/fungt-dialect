# fungt-dialect

MLIR dialect for shader compilation, lowering to SPIR-V for use in OpenGL
and, in the future, Vulkan.

## Status

`Vertex` and `Fragment` execution mode SPIR-V has been proven to round
trip correctly through MLIR's SPIR-V dialect, serialize with
`mlir-translate`, validate with `spirv-val`, and load in OpenGL via
`GL_ARB_gl_spirv` on both Intel and NVIDIA drivers. This was done with
hand written `spirv.module` MLIR, independent of fungt-dialect's own ops.
fungt-dialect's own ops and lowering passes do not yet emit `Vertex` or
`Fragment` modules.

## Overview

fungt-dialect defines the ops and lowering passes needed to build shaders
in MLIR and compile them to SPIR-V. It currently has no shading specific
ops. A shading language needs, at minimum, first class support for
samplers, images, varying and uniform storage classes, and interpolation
qualifiers, none of which fungt-dialect currently represents. This is the
active area of work.

## Tools

**fungt-opt** Optimizer driver. Runs lowering passes on MLIR input.

**fungt-parse** FunGT IR parser. Reads user friendly .fgt text files and
emits MLIR with fungt ops.

**fungt-translate** SPIR-V serializer. Converts SPIR-V dialect MLIR to
binary .spv files.

## Shading Pipeline (proof of concept, not yet wired into fungt-dialect)

```
Hand written spirv.module MLIR, Fragment or Vertex execution mode
        |
   mlir-translate --serialize-spirv --no-implicit-module -o shader.spv
        |
   spirv-val shader.spv
        |
   OpenGL: glShaderBinary + glSpecializeShader (GL_ARB_gl_spirv)
```

Both the vertex and fragment shaders in a single OpenGL program must be
SPIR-V modules together. Mixing a text compiled GLSL shader with a SPIR-V
specialized shader in the same program is invalid per the
`GL_ARB_gl_spirv` spec and produces `GL_INVALID_OPERATION` on link, and
was observed to cause a driver level segmentation fault in Mesa's `iris`
driver on Intel Arc hardware before the mixed state was corrected.

There is no surface syntax for shaders yet.

## Building

Requires an LLVM/MLIR build. Point CMake at your MLIR install:

```bash
mkdir build && cd build
cmake -G Ninja .. \
  -DMLIR_DIR=/path/to/llvm-project/build/lib/cmake/mlir \
  -DLLVM_EXTERNAL_LIT=/path/to/llvm-project/build/bin/llvm-lit
ninja
```

## Prior work

fungt-dialect originally targeted `GLCompute` execution mode SPIR-V for
LLM generated particle physics kernels, loaded via SYCL's
`kernel_compiler` extension. That work has been superseded by the
shading language direction above and is no longer the project's purpose.

## Part of FunGT

https://github.com/FunGTs/FunGT

## License

Apache 2.0 with LLVM Exceptions. See LICENSE for details.