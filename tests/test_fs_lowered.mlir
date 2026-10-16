module {
  spirv.module Logical GLSL450 requires #spirv.vce<v1.0, [Shader], []> {
    spirv.GlobalVariable @color bind(0, 0) : !spirv.ptr<vector<4xf32>, Uniform>
    spirv.GlobalVariable @out_var_0 {location = 0 : i32} : !spirv.ptr<vector<4xf32>, Output>
    spirv.func @main() "None" {
      %color_addr = spirv.mlir.addressof @color : !spirv.ptr<vector<4xf32>, Uniform>
      %0 = spirv.Load "Uniform" %color_addr : vector<4xf32>
      %out_var_0_addr = spirv.mlir.addressof @out_var_0 : !spirv.ptr<vector<4xf32>, Output>
      spirv.Store "Output" %out_var_0_addr, %0 : vector<4xf32>
      spirv.Return
    }
    spirv.EntryPoint "Fragment" @main, @out_var_0
    spirv.ExecutionMode @main "OriginUpperLeft"
  }
}

