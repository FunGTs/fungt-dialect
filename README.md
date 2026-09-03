# fungt-dialect

MLIR dialect for shader compilation, lowering to SPIR-V for use in OpenGL
and Vulkan.

## Status

fungt-dialect is a working shader compiler. A user writes a `.fgt` shader
file, runs three commands, and gets a validated `.spv` binary ready to
load into OpenGL or Vulkan. Confirmed rendering on both Intel Arc and
NVIDIA hardware.

## Pipeline

```
shader.fgt
        |
   fungt-parse --shader shader.fgt | fungt-opt --fungt-shader-lower-to-spirv - -o lowered.mlir
        |
   fungt-translate --fungt-to-spirv lowered.mlir -o shader.spv
        |
   OpenGL: glShaderBinary + glSpecializeShader (GL_ARB_gl_spirv)
   Vulkan: vkCreateShaderModule
```

## Surface Syntax

Users write `.fgt` files directly. No MLIR knowledge required.

Fragment shader reading from a uniform and writing to an output:

```
fragment_shader {
    uniform tint set(0) binding(0) : vec4
    color = load(tint)
    out color location(0)
}
```

Vertex shader passing an input through to an output:

```
vertex_shader {
    in position location(0) : vec4
    out gl_position location(0) : vec4
    gl_position = position
}
```

Supported statements inside a shader block:

`uniform name set(N) binding(N) : type` declares a uniform buffer binding.

`in name location(N) : type` reads from a vertex attribute or varying input.

`out name location(N)` writes a named value to a shader output.

`name = load(binding)` loads the value from a named uniform binding.

`name = expression` assigns the result of an expression to a variable.

Supported types: `vec4`

Comments use `//`.

## Ops

**fungt.shader_entry** Declares a shader entry point with an execution
model of `"Vertex"` or `"Fragment"`.

**fungt.shader_end** Terminates a shader entry body.

**fungt.const_vec4** Constant 4-component float vector.

**fungt.input** Reads from a location decorated shader input.

**fungt.output** Writes to a location decorated shader output.

**fungt.resource_binding** Declares a named resource at a descriptor set
and binding index. Storage class must be `"Uniform"`, `"StorageBuffer"`,
or `"UniformConstant"`.

**fungt.load_resource** Reads the value from a named resource binding.

## Tools

**fungt-parse** Reads a `.fgt` file and emits fungt-dialect MLIR. Use
`--shader` flag for shader files.

**fungt-opt** Runs lowering passes. `--fungt-shader-lower-to-spirv`
lowers fungt shader ops to `spirv` dialect MLIR. Text in, text out.

**fungt-translate** Serializes `spirv` dialect MLIR to a binary `.spv`
file. Text in, binary out.

## OpenGL Note

Both vertex and fragment shaders in a single OpenGL program must be
SPIR-V modules. Mixing GLSL and SPIR-V shaders in the same program
produces `GL_INVALID_OPERATION` on link, and causes a driver level
segmentation fault in Mesa on Intel Arc hardware.

## Building

Requires an LLVM/MLIR build. Point CMake at your MLIR install:

```bash
mkdir build && cd build
cmake -G Ninja .. \
  -DMLIR_DIR=/path/to/llvm-project/build/lib/cmake/mlir \
  -DLLVM_EXTERNAL_LIT=/path/to/llvm-project/build/bin/llvm-lit
ninja
```

## Prior Work

fungt-dialect originally targeted `GLCompute` execution mode SPIR-V for
LLM generated particle physics kernels. That work has been superseded by
the shading language direction above and is no longer the project's
purpose.

## Part of FunGT

https://github.com/FunGTs/FunGT

## License

Apache 2.0 with LLVM Exceptions. See LICENSE for details.