module {
  spirv.module Logical GLSL450 requires #spirv.vce<v1.0, [Shader], []> {
    spirv.GlobalVariable @out_var_1 {location = 0 : i32} : !spirv.ptr<vector<4xf32>, Output>
    spirv.GlobalVariable @in_var_0 {location = 0 : i32} : !spirv.ptr<vector<4xf32>, Input>
    spirv.func @main() "None" {
      %in_var_0_addr = spirv.mlir.addressof @in_var_0 : !spirv.ptr<vector<4xf32>, Input>
      %0 = spirv.Load "Input" %in_var_0_addr : vector<4xf32>
      %out_var_1_addr = spirv.mlir.addressof @out_var_1 : !spirv.ptr<vector<4xf32>, Output>
      spirv.Store "Output" %out_var_1_addr, %0 : vector<4xf32>
      spirv.Return
    }
    spirv.EntryPoint "Vertex" @main, @in_var_0, @out_var_1
  }
}

