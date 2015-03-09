#ifndef LUNA_FULLSCREENTRIANGLE_HLSL_INCLUDED
#define LUNA_FULLSCREENTRIANGLE_HLSL_INCLUDED

struct FullScreenTriangleVSOut
{
  float4 Position : SV_Position;
	float2 TexCoord : TEXCOORD0;
};

FullScreenTriangleVSOut FullScreenTriangleVS(uint vertexID : SV_VertexID)
{
	FullScreenTriangleVSOut output = (FullScreenTriangleVSOut)0;

	float2 grid = float2((vertexID << 1) & 2, vertexID & 2);
  output.Position = float4(grid * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	output.TexCoord = grid;//float2(vertexID%2,vertexID%4/2);

	return output;
}

#endif // LUNA_FULLSCREENTRIANGLE_HLSL_INCLUDED
