func.func @test_dist(%x0: f32, %y0: f32, %z0: f32,
                      %x1: f32, %y1: f32, %z1: f32) -> f32 {
  %d = fungt.distance %x0, %y0, %z0, %x1, %y1, %z1 : f32
  return %d : f32
}