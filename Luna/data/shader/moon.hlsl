#include "common.hlsli"

Texture2D gAlbedo : register(t0);
ByteAddressBuffer gVertices : register(t1);
RWStructuredBuffer<Vertex> gVerticesRW : register(u0);

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};

cbuffer CB : register(b1)
{
	float4x4 cWorldMatrix;
	uint cVertexCount;
}

[numthreads(256, 1, 1)]
void TransformCS(uint id : SV_DispatchThreadID)
{
	if (cVertexCount <= id) return;
	Vertex v;
	v.position = asfloat(gVertices.Load3(id*32));
	v.normal = asfloat(gVertices.Load3(id*32 + 12));
	v.uv = asfloat(gVertices.Load3(id*32 + 24));
	Vertex outV = gVerticesRW[id];
	outV.position = mul(float4(v.position, 1), cWorldMatrix);
	outV.normal = mul(float4(v.normal, 0), cWorldMatrix);
	outV.uv = v.uv;
	gVerticesRW[id] = outV;
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 pos = mul(float4(input.position, 1), cWorldMatrix);
	output.position = mul(pos, cViewProjectionMatrix);
	output.normal = mul(float4(input.normal, 1), cWorldMatrix).xyz;
	output.uv = input.uv;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 albedo = gAlbedo.Sample(gSamplerBilinearClamp, input.uv);
	return float4(albedo.rgb, 1);
}
