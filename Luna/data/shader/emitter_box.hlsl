#include "common.hlsli"
#include "particle.hlsli"

RWStructuredBuffer<Particle> gParticleBufferUAV : register(u0);
RWStructuredBuffer<uint4> gEmitterRandomSeed : register(u1);
ConsumeStructuredBuffer<uint> gConsumeDeadList : register(u2);
Buffer<uint> gDeadListCount : register(t0);

cbuffer EmitterCB : register(b1)
{
	float3 cEmitterPosition;
	float cEmitLifeTime;
	float3 cEmitterSize;
	uint cEmitCount;
	float4 cEmitterColor;
}

[numthreads(256, 1, 1)]
void EmitInBoxCS(uint id : SV_DispatchThreadID)
{
	if (cEmitCount <= id || gDeadListCount[5] <= id) return;
	
	uint4 r = gEmitterRandomSeed[id];
	uint4 r0 = xorshift(r);
	uint4 r1 = xorshift(r0);
	gEmitterRandomSeed[id] = r1;

	float u0 = pow(random(r0.x) * 2 - 1, 3);
	float u1 = pow(random(r1.x) * 2 - 1, 3);

	uint newIndex = gConsumeDeadList.Consume();
	Particle newParticle;
	newParticle.position = cEmitterPosition + float3(cEmitterSize.x*u0,0,cEmitterSize.z*u1);
	newParticle.velocity = float3(0, 0.5, 0);
	newParticle.lifeTime = cEmitLifeTime;
	newParticle.color = cEmitterColor;
//	newParticle.beginPosition = newParticle.position;
//	newParticle.beginNormal = float3(0, 1, 0);
	newParticle.universalIndex = 0;
	gParticleBufferUAV[newIndex] = newParticle;
}
