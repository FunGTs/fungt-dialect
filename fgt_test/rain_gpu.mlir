gpu.module @rain_kernels attributes {spirv.target_env = #spirv.target_env<#spirv.vce<v1.0, [Shader], [SPV_KHR_storage_buffer_storage_class]>, #spirv.resource_limits<>>} 
{
  gpu.func @rain_kernel(%particles: memref<9xf32, #spirv.storage_class<StorageBuffer>>, %dt: f32)
    kernel
    attributes {spirv.entry_point_abi = #spirv.entry_point_abi<workgroup_size = [64, 1, 1]>} {
    %c2 = arith.constant 2 : index
    %c5 = arith.constant 5 : index
    %c8 = arith.constant 8 : index
    %gravity = arith.constant -9.800000e+00 : f32

    %vel_z = memref.load %particles[%c5] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
    %pos_z = memref.load %particles[%c2] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
    %age = memref.load %particles[%c8] : memref<9xf32, #spirv.storage_class<StorageBuffer>>

    %dv = arith.mulf %gravity, %dt : f32
    %new_vz = arith.addf %vel_z, %dv : f32
    %new_pz = arith.addf %pos_z, %new_vz : f32
    %new_age = arith.addf %age, %dt : f32

    memref.store %new_vz, %particles[%c5] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
    memref.store %new_pz, %particles[%c2] : memref<9xf32, #spirv.storage_class<StorageBuffer>>
    memref.store %new_age, %particles[%c8] : memref<9xf32, #spirv.storage_class<StorageBuffer>>

    gpu.return
  }
}