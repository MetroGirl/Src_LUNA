#include "common.hlsli"
#include "fullScreenTriangle.hlsl"

Texture2D gShadowMap : register(t0);

// render floor
float4 FloorPS(FullScreenTriangleVSOut input) : SV_Target
{
	float4 rayDirection = float4(
	((((input.Position.x) * 2) / cResolution.x) - 1) / cProjectionMatrix[0][0],
	-((((input.Position.y) * 2) / cResolution.y) - 1) / cProjectionMatrix[1][1],
	1, 0);
	rayDirection = normalize(mul(rayDirection, cViewInverseMatrix));
	float4 rayOrigin = float4(cViewInverseMatrix[3][0], cViewInverseMatrix[3][1], cViewInverseMatrix[3][2], 1);
	float t = (-1 - rayOrigin.y)/rayDirection.y;
	float3 color = (float3)0.0;
	if (t > 0) {
		float3 p = rayOrigin.xyz + rayDirection.xyz*t;
		color = float3(0.7,0.7,0.7) * (exp(-0.05*t)*0.6);
	}
	return float4(color, 1);
}
