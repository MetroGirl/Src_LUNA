ID = "Shader"

shaders = {
  draw_floor = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/environment.hlsl", "FloorPS" },
  },
  moon = {
    vs = { "data/shader/moon.hlsl", "VS" },
    ps = { "data/shader/moon.hlsl", "PS" },
  },
  moon_transform = {
    cs = { "data/shader/moon.hlsl", "TransformCS" },
  },
}
