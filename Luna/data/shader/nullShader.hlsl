#include "common.hlsli"	

float4 DefaultShaderVS() : SV_POSITION
{
	return float4(0,0,0,0);
}

float4 DefaultShaderPS() : SV_TARGET0
{
	return float4(1,1,1,1);
}
