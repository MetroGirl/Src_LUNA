struct VS_INPUT
{
  float4 Position : POSITION;
	float4 Color : COLOR;
};

struct VS_OUTPUT
{
  float4 Position : SV_Position;
	float4 Color : COLOR;
};


VS_OUTPUT TestVS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = input.Position;
	output.Color = input.Color;

	return output;
}

float4 TestPS(VS_OUTPUT input) : SV_Target0
{
  return input.Color;
}


VS_OUTPUT Test2VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;

	output.Position = input.Position;

	return output;
}

float4 Test2PS(VS_OUTPUT input) : SV_Target0
{
  return float4(0,1,0,1);
}
