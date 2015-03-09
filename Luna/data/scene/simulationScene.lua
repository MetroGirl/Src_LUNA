ID = "Script"

Name = "simulationScene"

Start= 0
End  = 211.813995

Tasks = {
	{
		Type = "ParticleEmitterTask",
		Name = "MasterEmitter",
	},
	{
		Type = "FluidGridTask",
		Name = "FluidGrid",
	},
	{
		Type = "FloorTask", 
		Name = "floor",
	},
	{
		Type = "CameraTask",
		Name = "camera",
	},
}
