func.func @test_rain(%pos_x: f32, %pos_y: f32, %pos_z: f32,
                      %vel_x: f32, %vel_y: f32, %vel_z: f32,
                      %mass: f32, %dt: f32, %age: f32) {
  %results:7 = fungt.update(%pos_x, %pos_y, %pos_z,
                             %vel_x, %vel_y, %vel_z,
                             %mass, %dt, %age)
  {
  ^bb0(%px: f32, %py: f32, %pz: f32,
       %vx: f32, %vy: f32, %vz: f32,
       %m: f32, %t: f32, %a: f32):
    %gravity = arith.constant -9.8 : f32
    %dv = arith.mulf %gravity, %t : f32
    %new_vz = arith.addf %vz, %dv : f32
    %new_pz = arith.addf %pz, %new_vz : f32
    %new_age = arith.addf %a, %t : f32
      fungt.yield %px, %py, %new_pz, %vx, %vy, %new_vz, %new_age : f32, f32, f32, f32, f32, f32, f32
  } : (f32, f32, f32, f32, f32, f32, f32, f32, f32) -> (f32, f32, f32, f32, f32, f32, f32)
  return
}