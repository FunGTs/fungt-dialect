gpu.module @rain_kernels {
  gpu.func @rain_kernel(%pos_x: f32, %pos_y: f32, %pos_z: f32,
                         %vel_x: f32, %vel_y: f32, %vel_z: f32,
                         %mass: f32, %dt: f32, %age: f32)
    -> (f32, f32, f32, f32, f32, f32, f32)
    kernel {
    %cst = arith.constant -9.800000e+00 : f32
    %0 = arith.mulf %dt, %cst : f32
    %1 = arith.addf %vel_z, %0 : f32
    %2 = arith.addf %pos_z, %1 : f32
    %3 = arith.addf %age, %dt : f32
    gpu.return %pos_x, %pos_y, %2, %vel_x, %vel_y, %1, %3 : f32, f32, f32, f32, f32, f32, f32
  }
}