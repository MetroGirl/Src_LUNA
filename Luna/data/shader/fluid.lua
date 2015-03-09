ID = "Shader"

shaders = {
  particle_init = {
    cs = { "data/shader/particle.hlsl", "InitializeParticleCS" },
  },
	particle_copy_moon_vertex_dead = {
	  cs = { "data/shader/particle.hlsl", "CopyDeadMoonVertexCountCS" },
	},
  particle_emit_in_sphere = {
    cs = { "data/shader/particle.hlsl", "EmitInSphereCS" },
  },
  particle_emit_in_box = {
    cs = { "data/shader/emitter_box.hlsl", "EmitInBoxCS" },
  },
  particle_emit_moon = {
    cs = { "data/shader/particle.hlsl", "MoonEmitterCS" },
  },
  particle_simulate = {
    cs = { "data/shader/particle.hlsl", "SimulateParticleCS" },
  },
  particle_no_simulate = {
    cs = { "data/shader/particle.hlsl", "NoSimulateParticleCS" },
  },
  particle_simulate_a = {
    cs = { "data/shader/particle.hlsl", "SimulateParticleACS" },
  },
	particle_simulate_color = {
	  cs = { "data/shader/particle.hlsl", "SimulateParticleColorCS" },
	},
  particle_simulate_moon = {
    cs = { "data/shader/particle.hlsl", "SimulateParticleMoonCS" },
  },
  particle_sort = {
    cs = { "data/shader/particlesort.hlsl", "BitonicSortCS" },
  },
  particle_sort_transpose = {
    cs = { "data/shader/particlesort.hlsl", "MatrixTransposeCS" },
  },
  particle_render = {
    vs = { "data/shader/particle.hlsl", "VS" },
    gs = { "data/shader/particle.hlsl", "GS" },
    ps = { "data/shader/particle.hlsl", "PS" },
  },
  particle_moon_render = {
    vs = { "data/shader/particle.hlsl", "MoonVS" },
    gs = { "data/shader/particle.hlsl", "GS" },
    ps = { "data/shader/particle.hlsl", "PS" },
  },
  particle_shadow = {
    vs = { "data/shader/particleShadow.hlsl", "VS" },
    gs = { "data/shader/particleShadow.hlsl", "GS" },
  },
  fluid_advect = {
    cs = { "data/shader/fluid3d.hlsl", "AdvectCS" },
  },
  fluid_advect_backward = {
    cs = { "data/shader/fluid3d.hlsl", "AdvectBackwardCS" },
  },
  fluid_advect_maccormack = {
    cs = { "data/shader/fluid3d.hlsl", "AdvectMacCormackCS" },
  }, 
  fluid_divergence = {
    cs = { "data/shader/fluid3d.hlsl", "DivergenceCS" },
  }, 
  fluid_jacobi = {
    cs = { "data/shader/fluid3d.hlsl", "JacobiCS" },
  }, 
  fluid_project = {
    cs = { "data/shader/fluid3d.hlsl", "ProjectCS" },
  }, 
  fluid_add_pressure = {
    cs = { "data/shader/fluid3d.hlsl", "AddPressureCS" },
  },
  fluid_reset = {
    cs = { "data/shader/fluid3d.hlsl", "ResetCS" },
  },
  fluid_render_volume = {
    vs = { "data/shader/fluid3d.hlsl", "RenderVolumeVS" },
    ps = { "data/shader/fluid3d.hlsl", "RenderVolumePS" },
  },
  fluid_render_grid = {
    vs = { "data/shader/fluid3d.hlsl", "RenderVolumeVS" },
    ps = { "data/shader/fluid3d.hlsl", "RenderGridPS" },
  },
}
