# fungt-dialect

MLIR dialect for shader compilation, lowering to SPIR-V for use in OpenGL
and, in the future, Vulkan.

## Status

fungt-dialect can now express `Vertex` and `Fragment` shaders in its own
op syntax, lower them to `spirv.module` via `fungt-opt`, serialize to
binary `.spv` via `fungt-translate`, and load the result in OpenGL via
`GL_ARB_gl_spirv`. Validated with `spirv-val` and confirmed rendering on
both Intel Arc and NVIDIA hardware.

## Ops

**fungt.shader_entry** Declares a shader entry point. Takes a symbol name
and an execution model (`"Vertex"` or `"Fragment"`). Holds a region body
containing shader ops, terminated by `fungt.shader_end`.

**fungt.shader_end** Terminates a `shader_entry` body. No operands.

**fungt.const_vec4** Constant 4-component float vector. Takes four f32
attributes, produces a `vector<4xf32>` result.

**fungt.output** Writes a value to a location decorated shader output.
Takes a value and a non-negative location index.

**fungt.input** Reads a value from a location decorated shader input.
Takes a non-negative location index, produces a value.

**fungt.resource_binding** Declares a named shader resource at a given
descriptor set and binding index. Storage class must be `"Uniform"`,
`"StorageBuffer"`, or `"UniformConstant"`.

**fungt.load_resource** Reads the value from a named resource binding.
Takes a symbol reference to a `fungt.resource_binding`.

## Tools

**fungt-opt** Runs lowering passes on fungt-dialect MLIR text input,
producing `spirv` dialect MLIR text output. Text in, text out.

**fungt-translate** Serializes `spirv` dialect MLIR to a binary `.spv`
file. Text in, binary out.

**fungt-parse** FunGT IR parser. Not yet implemented for the shading
language surface syntax.

## Pipeline

```
fungt-dialect MLIR (hand written for now)
        |
   fungt-opt --fungt-shader-lower-to-spirv shader.mlir -o lowered.mlir
        |
   fungt-translate --fungt-to-spirv lowered.mlir -o shader.spv
        |
   spirv-val shader.spv
        |
   OpenGL: glShaderBinary + glSpecializeShader (GL_ARB_gl_spirv)
   Vulkan: vkCreateShaderModule
```

Both vertex and fragment shaders in a single OpenGL program must be
SPIR-V modules. Mixing GLSL and SPIR-V shaders in the same program
produces `GL_INVALID_OPERATION` on link, and was observed to cause a
driver level segmentation fault in Mesa on Intel Arc hardware.

## Example

Solid red fragment shader using a uniform color binding:

```mlir
module {
  fungt.resource_binding @tint storage_class("Uniform")
    layout(set = 0, binding = 0) : vector<4xf32>

  fungt.shader_entry @red_rect execution_model("Fragment") {
    %val = fungt.load_resource @tint : vector<4xf32>
    fungt.output %val location(0) : vector<4xf32>
    fungt.shader_end
  }
}
```

## Remaining Work

There is no surface syntax for shaders yet. All fungt-dialect MLIR is
currently hand written. `fungt-parse` for the shading language is the
next major piece of work.

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
LLM generated particle physics kernels. That work has been superseded by
the shading language direction above and is no longer the project's
purpose.

## Part of FunGT

https://github.com/FunGTs/FunGT

## License

Apache 2.0 with LLVM Exceptions. See LICENSE for details.