func.func @test_mul(%a: f32, %b: f32) -> f32 {
  %result = fungt.scalar_mul %a, %b : f32
  return %result : f32
}