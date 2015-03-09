#include "particle.hlsli"

StructuredBuffer<Particle> gParticleBufferSRV : register(t0);
StructuredBuffer<ParticleIndex> gParticleIndexBufferSRV : register(t1);

cbuffer ParticleShadowCB : register(b1)
{
	float4x4 cLightViewMatrix;
	float4x4 cLightProjectionMatrix;
}

struct VS_OUTPUT
{
	float3 position : POSITION;
};

struct GS_OUTPUT
{
	float4 position : SV_Position;
};

VS_OUTPUT VS(uint vertexID : SV_VertexID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	ParticleIndex key = gParticleIndexBufferSRV[vertexID];
	Particle p = gParticleBufferSRV[key.index];
	output.position = p.position;
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

	GS_OUTPUT output;
	float4 viewPos = mul(float4(input[0].position, 1), cLightViewMatrix);
	[unroll]
	for (int i = 0; i < 4; ++i) {
		output.position = mul(viewPos + offset[i]*0.1, cLightProjectionMatrix);
		stream.Append(output);
	}
	stream.RestartStrip();
}
