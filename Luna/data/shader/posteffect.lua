ID = "Shader"

shaders = {
  posteffect_white = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "WhitePostEffectPS" },
  },
  radialBlur = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "RadialBlurPS" },
  },
  noise = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "NoisePS" },
  },
  chromaticAberration = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "ChromaticAberrationPS" },
  },
  vignetting = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "VignettingPS" },
  },
  filmGrain = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "FilmGrainPS" },
  },
  copyAsIs = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "CopyAsIsPS" },
  },
  finalCombiner = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "FinalCombinerPS" },
  },
  fakeGamma = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "FakeGammaPS" },
  },
  bigScreenGamma = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "BigScreenGammaPS" },
  },
  loader = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "LoaderPS" },
  },
  exitFade = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "ExitFadePS" },
  },
  debugDispCapture = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "DebugDispCapturePS" },
  },
  vignetting2 = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "ColorBleedingPS" },
  },
  greetz = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "GreetzPS" },
  },
  greetzIntro = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "GreetzIntroPS" },
  },
  introScreen = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "IntroScreenPS" },
  },
  outroScreen = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "OutroScreenPS" },
  },
  helloScreen = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "HelloScreenPS" },
  },
  colorCorrection = {
    vs = { "data/shader/fullScreenTriangle.hlsl", "FullScreenTriangleVS" },
    ps = { "data/shader/postEffect.hlsl", "ColorCorrectionPS" },
  },
}
