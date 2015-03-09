#ifndef PARTICLE_HLSLI
#define PARTICLE_HLSLI

struct ParticleIndex
{
	float viewDistanceSq; // sort key
	uint index;
};

struct Particle
{
	float3 position;
	float3 velocity;
	float lifeTime;
	float4 color;
//	float3 beginPosition;
//	float3 beginNormal;
	uint universalIndex;
};

struct Particle2nd
{
	float3 beginPosition;
	float3 beginNormal;
};

#endif
