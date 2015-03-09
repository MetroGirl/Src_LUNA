#include "fullScreenTriangle.hlsl"

Texture2D SamplerObjectSampleTask : register( t0 );

SamplerState SamplerStateSampleTask {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
	AddressW = WRAP;
	MipmapLODBias = 0; 
	MaxAnisotropy = 1;
	ComparisonFunc = NEVER;
	MinLOD = 0;
	MaxLOD = FLOAT32_MAX;
};

float4 SampleTaskPS(FullScreenTriangleVSOut input) : SV_Target0
{
	discard;
  return float4(1,1,1,0);//SamplerObjectSampleTask.Load( input.Position );
}
