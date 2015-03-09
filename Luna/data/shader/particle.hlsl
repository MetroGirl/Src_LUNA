#include "common.hlsli"
#include "particle.hlsli"

RWStructuredBuffer<Particle> gParticleBufferUAV : register(u0);
RWStructuredBuffer<ParticleIndex> gParticleIndexBufferUAV : register(u1);
//AppendStructuredBuffer<uint> gAppendDeadList : register(u2);
RWBuffer<uint> gAppendDeadList : register(u2);
//ConsumeStructuredBuffer<uint> gConsumeDeadList : register(u3);
RWStructuredBuffer<uint4> gEmitterRandomSeed : register(u4);
RWBuffer<uint> gAppendMoonVertex : register(u5);
//RWBuffer<uint> gConsumeMoonVertex : register(u6);
RWStructuredBuffer<Particle2nd> gParticle2ndBufferUAV : register(u6);
RWBuffer<uint> gMoonVertexCounter : register(u7);

StructuredBuffer<Particle> gParticleBufferSRV : register(t0);
StructuredBuffer<ParticleIndex> gParticleIndexBufferSRV : register(t1);
Texture2D gParticleTexture : register(t2);
Texture3D<float4> gFluidVelocity : register(t3);
Texture2D<float> gShadowMap : register(t4);
Buffer<uint> gDeadListCount : register(t5);
StructuredBuffer<Vertex> gMoonVertexBuffer : register(t6);
Texture2D gMoonTexture : register(t7);

SamplerState gShadowSampler : register(s0);

cbuffer EmitterCB : register(b1)
{
	float3 cEmitterPosition;
	float cEmitterRadius;
	float4 cEmitterColor;
	uint cEmitCount;
	float cEmitLifeTime;
	uint cUniversalIndex;
	float _;
}

cbuffer FluidCB : register(b2)
{
	float4 cGridPositionMin;
	float4 cGridSize;
}

cbuffer InitCB : register(b3)
{
	uint cMoonVertexCount;
}

float3 sampleVelocity(float3 position)
{
	float3 gridPosition = ((position - cGridPositionMin.xyz) / cGridSize.xyz);
	float4 velocity = gFluidVelocity.SampleLevel(gSamplerBilinearClamp, gridPosition, 0);
	return velocity.xyz;
}

[numthreads(256, 1, 1)]
void InitializeParticleCS(uint id : SV_DispatchThreadID)
{
	//gAppendDeadList.Append(id);
	InterlockedAdd(gMoonVertexCounter[2], 1);
	gAppendDeadList[id] = id;
//	if (id < cMoonVertexCount) {
		//gAppendMoonVertex.IncrementCounter();
	InterlockedAdd(gMoonVertexCounter[0], 1);
	gAppendMoonVertex[id] = id % cMoonVertexCount;//.Append(id);
//	}
}

[numthreads(1, 1, 1)]
void CopyDeadMoonVertexCountCS(uint id : SV_DispatchThreadID)
{
	gMoonVertexCounter[1] = gMoonVertexCounter[0];
	gMoonVertexCounter[3] = gMoonVertexCounter[2];
}

[numthreads(256, 1, 1)]
void EmitInSphereCS(uint id : SV_DispatchThreadID)
{
	if (cEmitCount <= id || gMoonVertexCounter[3] <= id/*gDeadListCount[5] <= id*/) return;

	uint4 r = gEmitterRandomSeed[id];
	uint4 r0 = xorshift(r);
	uint4 r1 = xorshift(r0);
	uint4 r2 = xorshift(r1);
	uint4 r3 = xorshift(r2);
	gEmitterRandomSeed[id] = r3;

	float u0 = random(r0.x);
	float u1 = random(r1.x) * 2 - 1;
	float u2 = random(r2.x) * 2 * 3.14159;
	float x = pow(u0,1/3) * sqrt(1-u1*u1) * cos(u2) * cEmitterRadius;
	float y = pow(u0,1/3) * sqrt(1-u1*u1) * sin(u2) * cEmitterRadius;
	float z = pow(u0,1/3) * u1 * cEmitterRadius;

	uint counter;
	InterlockedAdd(gMoonVertexCounter[2], -1, counter);
	uint newIndex = gAppendDeadList[counter];//gConsumeDeadList.Consume();

	float mag = 0;
	[unroll]
	for (uint i = 0; i < 4; ++i) {
		mag += cSoundFFTBand[(cUniversalIndex-1)*2 + i];
	}

	float4 color = cEmitterColor;
	if (cUniversalIndex > 0) {
		float h = clamp((cUniversalIndex-1) / 9.0, 0, 1);
		color = HSVtoRGB(float4(h, 0.8, clamp(mag*0.2 , 0, 1), color.a));
	}
	float velocityVariance = random(r3.x)*2 - 1;

	Particle newParticle;
	newParticle.position = cEmitterPosition + float3(x,y,z);
	newParticle.velocity = float3(0, (mag + velocityVariance)*0.07  , 0);
	newParticle.lifeTime = cEmitLifeTime;
	newParticle.color = color;
//	newParticle.beginPosition = newParticle.position;
//	newParticle.beginNormal = normalize(float3(x,y,z) - cEmitterPosition);
	newParticle.universalIndex = 0;
	gParticleBufferUAV[newIndex] = newParticle;

	Particle2nd newParticle2nd;
	newParticle2nd.beginPosition = newParticle.position;
	newParticle2nd.beginNormal = normalize(newParticle.position - cEmitterPosition);
	gParticle2ndBufferUAV[newIndex] = newParticle2nd;
}

[numthreads(256, 1, 1)]
void MoonEmitterCS(uint id : SV_DispatchThreadID)
{
	if (cEmitCount <= id || gMoonVertexCounter[3] <= id /*gDeadListCount[5] <= id*//* || gMoonVertexCounter[1] <= id*/) return;

	uint4 r = gEmitterRandomSeed[id];
	uint4 r0 = xorshift(r);
	uint4 r1 = xorshift(r0);
	uint4 r2 = xorshift(r1);
	uint4 r3 = xorshift(r2);
	uint4 r4 = xorshift(r3);
	gEmitterRandomSeed[id] = r4;

	float u0 = random(r0.x);
	float u1 = random(r1.x) * 2 - 1;
	float u2 = random(r2.x) * 2 * 3.14159;
	float x = pow(u0,1/3) * sqrt(1-u1*u1) * cos(u2) * cEmitterRadius;
	float y = pow(u0,1/3) * sqrt(1-u1*u1) * sin(u2) * cEmitterRadius;
	float z = pow(u0,1/3) * u1 * cEmitterRadius;

	
	uint count;
	InterlockedAdd(gMoonVertexCounter[2], -1, count);
	//uint newIndex = gConsumeDeadList.Consume();
	uint newIndex = gAppendDeadList[count];
	InterlockedAdd(gMoonVertexCounter[0], -1, count);
	//uint counter = gConsumeMoonVertex.DecrementCounter();
	uint targetIndex = gAppendMoonVertex[count/*%cMoonVertexCount*/];//gConsumeMoonVertex[counter];//gConsumeMoonVertex[counter];//.Consume();
	Vertex v = gMoonVertexBuffer[targetIndex];

	float3 newPosition = cEmitterPosition + float3(x, y, z);
	float3 newVelocity = ((v.position - newPosition) + (random(r3)-0.5)*2);

	Particle newParticle;
	newParticle.position = newPosition;
	newParticle.velocity = newVelocity;
	newParticle.lifeTime = cEmitLifeTime + (random(r4.x)-0.5)*40;
	newParticle.color = cEmitterColor;
//	newParticle.beginPosition = newParticle.position;
//	newParticle.beginNormal = float3(0,0,0);//normalize(float3(x,y,z) - cEmitterPosition);
	newParticle.universalIndex = targetIndex + 1;
	gParticleBufferUAV[newIndex] = newParticle;
}

// no simulation
[numthreads(256, 1, 1)]
void NoSimulateParticleCS(uint id : SV_DispatchThreadID)
{
	Particle p = gParticleBufferUAV[id];

	if (p.lifeTime <= 0) {
		gParticleIndexBufferUAV[id].viewDistanceSq = -1;
		return;
	}

	p.lifeTime -= 1;
	if (p.lifeTime < 1) {
		p.lifeTime = -1;
		gParticleBufferUAV[id] = p;
		int counter;
		InterlockedAdd(gMoonVertexCounter[2], 1, counter);
		gAppendDeadList[counter] = id;
//		gAppendDeadList.Append(id);
		return;
	}

	Particle2nd p2 = gParticle2ndBufferUAV[id];

	float3 viewPosition = mul(float4(p.position, 1), cViewMatrix).xyz;
	float3 viewVec = viewPosition - cEyePosition.xyz;
	float viewDistanceSq = dot(viewVec, viewVec);

	uint index = gParticleIndexBufferUAV.IncrementCounter();
	ParticleIndex key;
	key.index = id;
	key.viewDistanceSq = viewDistanceSq;

	gParticleBufferUAV[id] = p;
	gParticleIndexBufferUAV[index] = key;
}

// sound simulator
[numthreads(256, 1, 1)]
void SimulateParticleACS(uint id : SV_DispatchThreadID)
{
	Particle p = gParticleBufferUAV[id];

	if (p.lifeTime <= 0) {
		gParticleIndexBufferUAV[id].viewDistanceSq = -1;
		return;
	}

	Particle2nd p2 = gParticle2ndBufferUAV[id];

	float mag = 0;
	[unroll]
	for (uint i = 0; i < 8; ++i) {
		mag += cSoundFFTBand[i];
	}

	float3 viewPosition = mul(float4(p.position, 1), cViewMatrix).xyz;
	float3 viewVec = viewPosition - cEyePosition.xyz;
	float viewDistanceSq = dot(viewVec, viewVec);

	p.position = p2.beginPosition + p2.beginNormal*mag*0.1;

	uint index = gParticleIndexBufferUAV.IncrementCounter();
	ParticleIndex key;
	key.index = id;
	key.viewDistanceSq = viewDistanceSq;

	gParticleBufferUAV[id] = p;
	gParticleIndexBufferUAV[index] = key;
}

// moon constraint
[numthreads(256, 1, 1)]
void SimulateParticleMoonCS(uint id : SV_DispatchThreadID)
{
	Particle p = gParticleBufferUAV[id];

	if (p.lifeTime <= 0) {
		gParticleIndexBufferUAV[id].viewDistanceSq = -1;
		return;
	} 

	float elapsed = cDemoTimeSecond - 155;

	float3 viewPosition = mul(float4(p.position, 1), cViewMatrix).xyz;
	float3 viewVec = viewPosition - cEyePosition.xyz;
	float viewDistanceSq = dot(viewVec, viewVec);

	float3 moonGravity = (float3)0;
	if (0 < p.universalIndex) {
		uint targetIndex = p.universalIndex - 1;
		Vertex v = gMoonVertexBuffer[targetIndex];
		moonGravity = (v.position - p.position)*0.05*max(elapsed-15, 0);
	}

	float3 fluidVelocity = sampleVelocity(p.position);
	float3 totalVelocity = (p.velocity + moonGravity + fluidVelocity) * 0.03333333;
	p.position = p.position + totalVelocity;
	p.velocity = totalVelocity;
	
	p.lifeTime -= 1;
	if (p.lifeTime < 1) {
		p.lifeTime = -1;
		gParticleBufferUAV[id] = p;
		uint counter;
		InterlockedAdd(gMoonVertexCounter[2], 1, counter);
//		gAppendDeadList.Append(id);
		gAppendDeadList[counter] = id;
		if (p.universalIndex > 0) {

			InterlockedAdd(gMoonVertexCounter[0], 1, counter);
//			uint counter = gAppendMoonVertex.IncrementCounter();//.Append(p.universalIndex - 1);
			gAppendMoonVertex[counter] = p.universalIndex-1;
		}
		return;
	}

	uint index = gParticleIndexBufferUAV.IncrementCounter();
	ParticleIndex key;
	key.index = id;
	key.viewDistanceSq = viewDistanceSq;

	gParticleBufferUAV[id] = p;
	gParticleIndexBufferUAV[index] = key;
}

// sample fluid grid simulator
[numthreads(256, 1, 1)]
void SimulateParticleCS(uint id : SV_DispatchThreadID)
{
	Particle p = gParticleBufferUAV[id];
	if (p.lifeTime < 0) {
		gParticleIndexBufferUAV[id].viewDistanceSq = -1;
		return;
	}

	float3 viewPosition = mul(float4(p.position, 1), cViewMatrix).xyz;
	float3 viewVec = viewPosition - cEyePosition.xyz;
	float viewDistanceSq = dot(viewVec, viewVec);

	if (p.lifeTime < 240) {
		float3 velocity = sampleVelocity(p.position);
		p.position = p.position + velocity * 0.03333333;
	}
	p.lifeTime -= 1;
	if (p.lifeTime < 1) {
		p.lifeTime = -1;
		gParticleBufferUAV[id] = p;
		int counter;
		InterlockedAdd(gMoonVertexCounter[2], 1, counter);
		gAppendDeadList[counter] = id;
//		gAppendDeadList.Append(id);
		return;
	}

	float mag = 0;
	[unroll]
	for (uint i = 0; i < 8; ++i) {
		mag += cSoundFFTBand[i];
	}

	Particle2nd p2 = gParticle2ndBufferUAV[id];
	if (p.lifeTime > 240) {
		p.position = p2.beginPosition + p2.beginNormal*mag*0.05;
	}

	uint index = gParticleIndexBufferUAV.IncrementCounter();
	ParticleIndex key;
	key.index = id;
	key.viewDistanceSq = viewDistanceSq;

	gParticleBufferUAV[id] = p;
	gParticleIndexBufferUAV[index] = key;
}

// color particle simulator
[numthreads(256, 1, 1)]
void SimulateParticleColorCS(uint id : SV_DispatchThreadID)
{
	Particle p = gParticleBufferUAV[id];
	if (p.lifeTime < 0) {
		gParticleIndexBufferUAV[id].viewDistanceSq = -1;
		return;
	}

	float3 viewPosition = mul(float4(p.position, 1), cViewMatrix).xyz;
	float3 viewVec = viewPosition - cEyePosition.xyz;
	float viewDistanceSq = dot(viewVec, viewVec);

	float3 newVelocity = p.velocity;
	if (p.lifeTime < 240) {
		float blendRate = max((p.lifeTime - 120) / 120, 0);
		float3 fluidVelocity = sampleVelocity(p.position);
		newVelocity = (blendRate*p.velocity + (1-blendRate)*fluidVelocity);
	}
	p.position = p.position + newVelocity * 0.03333333;
	p.velocity = newVelocity;
	p.lifeTime -= 1;
	if (p.lifeTime < 1) {
		p.lifeTime = -1;
		gParticleBufferUAV[id] = p;
		int counter;
		InterlockedAdd(gMoonVertexCounter[2], 1, counter);
		gAppendDeadList[counter] = id;
//		gAppendDeadList.Append(id);
		return;
	}

	float mag = 0;
	[unroll]
	for (uint i = 0; i < 8; ++i) {
		mag += cSoundFFTBand[i];
	}

	Particle2nd p2 = gParticle2ndBufferUAV[id];

	uint index = gParticleIndexBufferUAV.IncrementCounter();
	ParticleIndex key;
	key.index = id;
	key.viewDistanceSq = viewDistanceSq;

	gParticleBufferUAV[id] = p;
	gParticleIndexBufferUAV[index] = key;
}

struct VS_OUTPUT
{
	float3 position : POSITION;
	float4 color : COLOR;
};

struct GS_OUTPUT
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

VS_OUTPUT VS(uint vertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	ParticleIndex key = gParticleIndexBufferSRV[vertexID];
	Particle p = gParticleBufferSRV[key.index];
	output.position = p.position;
	p.color.a *= min(p.color.a*0.5, p.lifeTime * 0.0003);
	output.color = p.color;
	return output;
}

VS_OUTPUT MoonVS(uint vertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	ParticleIndex key = gParticleIndexBufferSRV[vertexID];
	Particle p = gParticleBufferSRV[key.index];
	if (p.universalIndex > 0) {
		uint targetIndex = p.universalIndex - 1;
		Vertex v = gMoonVertexBuffer[targetIndex];
		v.uv.y = 1 - v.uv.y;
		uint2 pos = float2(4096, 2048) * v.uv;
		float4 moonColor = gMoonTexture[pos];
		p.color.rgb = moonColor.rgb * moonColor.rgb;
	}
	output.position = p.position;
	p.color.a *= min(p.color.a*0.5, p.lifeTime * 0.0003);
	output.color = p.color;
//	output.color.rgb = float3(1, 0, 0);
	return output;
}

[maxvertexcount(4)]
void GS(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> stream)
{
	const float4 offset[4] = {
    {-1, 1, 0, 0},
    { 1, 1, 0, 0},
    {-1,-1, 0, 0},
    { 1,-1, 0, 0},
	};

	const float2 uv[4] = {
		{0,0},{0,1},{1,0},{1,1}
	};

	// test fft
	float mag = 1;
#if 0
	[unroll]
	for (uint i = 0; i < 8; ++i) {
		mag += cSoundFFTBand[i];
	}
	mag -= 1;
#endif

	GS_OUTPUT output;
	float4 viewPos = mul(float4(input[0].position, 1), cViewMatrix);
	[unroll]
	for (int i = 0; i < 4; ++i) {
		output.position = mul(viewPos + offset[i]*0.01*mag, cProjectionMatrix);
		output.color = input[0].color;
		output.uv = uv[i];
		stream.Append(output);
	}
	stream.RestartStrip();
}

float4 PS(GS_OUTPUT input) : SV_Target0
{
	float alpha = gParticleTexture.Sample(gSamplerBilinearClamp, input.uv).a;
	return float4(input.color.rgb, input.color.a);
}
