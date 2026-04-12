module {
  spirv.module @__spv__rain_kernels Logical GLSL450 requires #spirv.vce<v1.0, [Shader], [SPV_KHR_storage_buffer_storage_class]> attributes {spirv.target_env = #spirv.target_env<#spirv.vce<v1.0, [Shader], [SPV_KHR_storage_buffer_storage_class]>, #spirv.resource_limits<>>} {
    spirv.GlobalVariable @rain_kernel_arg_0 bind(0, 0) : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>
    spirv.GlobalVariable @rain_kernel_arg_1 bind(0, 1) : !spirv.ptr<!spirv.struct<(f32 [0])>, StorageBuffer>
    spirv.func @rain_kernel() "None" attributes {workgroup_attributions = 0 : i64} {
      %rain_kernel_arg_0_addr = spirv.mlir.addressof @rain_kernel_arg_0 : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>
      %rain_kernel_arg_1_addr = spirv.mlir.addressof @rain_kernel_arg_1 : !spirv.ptr<!spirv.struct<(f32 [0])>, StorageBuffer>
      %cst0_i32 = spirv.Constant 0 : i32
      %0 = spirv.AccessChain %rain_kernel_arg_1_addr[%cst0_i32] : !spirv.ptr<!spirv.struct<(f32 [0])>, StorageBuffer>, i32 -> !spirv.ptr<f32, StorageBuffer>
      %1 = spirv.Load "StorageBuffer" %0 : f32
      %cst2_i32 = spirv.Constant 2 : i32
      %cst5_i32 = spirv.Constant 5 : i32
      %cst8_i32 = spirv.Constant 8 : i32
      %cst_f32 = spirv.Constant -9.800000e+00 : f32
      %cst0_i32_0 = spirv.Constant 0 : i32
      %cst0_i32_1 = spirv.Constant 0 : i32
      %cst1_i32 = spirv.Constant 1 : i32
      %2 = spirv.AccessChain %rain_kernel_arg_0_addr[%cst0_i32_0, %cst5_i32] : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>, i32, i32 -> !spirv.ptr<f32, StorageBuffer>
      %3 = spirv.Load "StorageBuffer" %2 : f32
      %cst0_i32_2 = spirv.Constant 0 : i32
      %cst0_i32_3 = spirv.Constant 0 : i32
      %cst1_i32_4 = spirv.Constant 1 : i32
      %4 = spirv.AccessChain %rain_kernel_arg_0_addr[%cst0_i32_2, %cst2_i32] : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>, i32, i32 -> !spirv.ptr<f32, StorageBuffer>
      %5 = spirv.Load "StorageBuffer" %4 : f32
      %cst0_i32_5 = spirv.Constant 0 : i32
      %cst0_i32_6 = spirv.Constant 0 : i32
      %cst1_i32_7 = spirv.Constant 1 : i32
      %6 = spirv.AccessChain %rain_kernel_arg_0_addr[%cst0_i32_5, %cst8_i32] : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>, i32, i32 -> !spirv.ptr<f32, StorageBuffer>
      %7 = spirv.Load "StorageBuffer" %6 : f32
      %8 = spirv.FMul %1, %cst_f32 : f32
      %9 = spirv.FAdd %3, %8 : f32
      %10 = spirv.FAdd %5, %9 : f32
      %11 = spirv.FAdd %7, %1 : f32
      %cst0_i32_8 = spirv.Constant 0 : i32
      %cst0_i32_9 = spirv.Constant 0 : i32
      %cst1_i32_10 = spirv.Constant 1 : i32
      %12 = spirv.AccessChain %rain_kernel_arg_0_addr[%cst0_i32_8, %cst5_i32] : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>, i32, i32 -> !spirv.ptr<f32, StorageBuffer>
      spirv.Store "StorageBuffer" %12, %9 : f32
      %cst0_i32_11 = spirv.Constant 0 : i32
      %cst0_i32_12 = spirv.Constant 0 : i32
      %cst1_i32_13 = spirv.Constant 1 : i32
      %13 = spirv.AccessChain %rain_kernel_arg_0_addr[%cst0_i32_11, %cst2_i32] : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>, i32, i32 -> !spirv.ptr<f32, StorageBuffer>
      spirv.Store "StorageBuffer" %13, %10 : f32
      %cst0_i32_14 = spirv.Constant 0 : i32
      %cst0_i32_15 = spirv.Constant 0 : i32
      %cst1_i32_16 = spirv.Constant 1 : i32
      %14 = spirv.AccessChain %rain_kernel_arg_0_addr[%cst0_i32_14, %cst8_i32] : !spirv.ptr<!spirv.struct<(!spirv.array<9 x f32, stride=4> [0])>, StorageBuffer>, i32, i32 -> !spirv.ptr<f32, StorageBuffer>
      spirv.Store "StorageBuffer" %14, %11 : f32
      spirv.Return
    }
    spirv.EntryPoint "GLCompute" @rain_kernel
    spirv.ExecutionMode @rain_kernel "LocalSize", 64, 1, 1
  }
  gpu.module @rain_kernels attributes {spirv.target_env = #spirv.target_env<#spirv.vce<v1.0, [Shader], [SPV_KHR_storage_buffer_storage_class]>, #spirv.resource_limits<>>} {
    gpu.func @rain_kernel(%arg0: memref<9xf32, #spirv.storage_class<StorageBuffer>>, %arg1: f32) kernel attributes {spirv.entry_point_abi = #spirv.entry_point_abi<workgroup_size = [64, 1, 1]>} {
      %c2 = arith.constant 2 : index
      %c5 = arith.constant 5 : index
      %c8 = arith.constant 8 : index
      %cst = arith.constant -9.800000e+00 : f32
      %0 = memref.load %arg0[%c5] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
      %1 = memref.load %arg0[%c2] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
      %2 = memref.load %arg0[%c8] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
      %3 = arith.mulf %cst, %arg1 : f32
      %4 = arith.addf %0, %3 : f32
      %5 = arith.addf %1, %4 : f32
      %6 = arith.addf %2, %arg1 : f32
      memref.store %4, %arg0[%c5] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
      memref.store %5, %arg0[%c2] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
      memref.store %6, %arg0[%c8] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
      gpu.return
    }
  }
}

