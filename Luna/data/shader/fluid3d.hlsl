#include "common.hlsli"

Texture3D<float4>   VelocityGrid0    : register(t0);
Texture3D<float4>   VelocityGrid1    : register(t1);
Texture3D<float4>   VelocityGrid2    : register(t2);
Texture3D<float>    Divergence       : register(t3);
Texture3D<float>    Pressure         : register(t4);
Texture3D<float4>   RotationGrid     : register(t5);
RWTexture3D<float4> VelocityGrid0_RW : register(u0);
RWTexture3D<float4> VelocityGrid1_RW : register(u1);
RWTexture3D<float4> VelocityGrid2_RW : register(u2);
RWTexture3D<float>  Divergence_RW    : register(u3);
RWTexture3D<float>  Pressure_RW      : register(u4);
RWTexture3D<float4> RotationGrid_RW  : register(u5);

struct PressureSource
{
	float3 position;
	float radius;
	float pressure;
};
StructuredBuffer<PressureSource> gPressureSources : register(t6);

SamplerState gLinearSampler : register(s0);
//SamplerState PointSampler  : register(s1);

cbuffer per_frame_constants : register(b1)
{
  uint4    Dimension;
	float4   DimensionInv;
  float4   GridSize;
	float4   cGridPositionMin;
	float4   cMovingVelocity;
  float    Timestep;
  bool     UseVorticityConfinement;
  float    VorticityCoeff;
	uint     cPressureSourceCount;
};

cbuffer volume_render_constants : register(b2)
{
	float4x4 WorldMatrix;
}

float3 vorticity_confinement(uint3 i);

float3 cell2texcoord(float3 cell)
{
  return (cell + 0.5) * DimensionInv.xyz;
}

// VelocityGrid0 -> VelocityGrid1
[numthreads(16, 4, 4)]
void AdvectCS(uint3 i : SV_DispatchThreadID)
{
  float3 position = i - VelocityGrid0[i].xyz;// * Timestep;
  float3 sample   = cell2texcoord(position);
  VelocityGrid1_RW[i]    = VelocityGrid0.SampleLevel(gLinearSampler, sample, 0);
}

// VelocityGrid0, VelocityGrid1 -> VelocityGrid2
[numthreads(16, 4, 4)]
void AdvectBackwardCS(uint3 i : SV_DispatchThreadID)
{
  float3 position = i + VelocityGrid0[i].xyz;// * Timestep;
  float3 sample   = cell2texcoord(position);
  VelocityGrid2_RW[i]    = VelocityGrid1.SampleLevel(gLinearSampler, sample, 0);
}

// VelocityGrid0, VelocityGrid2 -> VelocityGrid1
[numthreads(16, 4, 4)]
void AdvectMacCormackCS(uint3 i : SV_DispatchThreadID)
{
  float3 position = i - VelocityGrid0[i].xyz;// * Timestep;
  uint3  p = (uint3)position;
  float4 r0 = VelocityGrid0[p + uint3(0,0,0)];
  float4 r1 = VelocityGrid0[p + uint3(1,0,0)];
  float4 r2 = VelocityGrid0[p + uint3(0,1,0)];
  float4 r3 = VelocityGrid0[p + uint3(1,1,0)];
  float4 r4 = VelocityGrid0[p + uint3(0,0,1)];
  float4 r5 = VelocityGrid0[p + uint3(1,0,1)];
  float4 r6 = VelocityGrid0[p + uint3(0,1,1)];
  float4 r7 = VelocityGrid0[p + uint3(1,1,1)];

  float4 lmin = min(min(min(min(min(min(min(r0,r1),r2),r3),r4),r5),r6),r7);
  float4 lmax = max(max(max(max(max(max(max(r0,r1),r2),r3),r4),r5),r6),r7);

  float3 sample = cell2texcoord(position);
  float4 s0 = VelocityGrid0.SampleLevel(gLinearSampler, sample, 0);
  float4 s  = s0 + 0.5*(s0-VelocityGrid2.SampleLevel(gLinearSampler, sample, 0));
  s = clamp(s, lmin, lmax);
  //s.w = max(s0.w - 0.001, 0);
	float3 movingVelocity = cDemoTimeSecond < 150? cMovingVelocity: (float3)0;
  VelocityGrid1_RW[i] = s - cMovingVelocity;
	// TEST
#if 0
  if (i.y == 20 && 2 < i.x && i.x < 10 && 30 < i.z && i.z < 40) {
    VelocityGrid1_RW[i] = s + float4(0, 3.f, 0, 0);
  };
#endif
}

// VelocityGrid1 -> Divergence
[numthreads(16, 4, 4)]
void DivergenceCS(uint3 index : SV_DispatchThreadID)
{
  float4 pxm = VelocityGrid0[index + int3(-1, 0, 0)];
  float4 pxp = VelocityGrid0[index + int3( 1, 0, 0)];
  float4 pym = VelocityGrid0[index + int3( 0,-1, 0)];
  float4 pyp = VelocityGrid0[index + int3( 0, 1, 0)];
  float4 pzm = VelocityGrid0[index + int3( 0, 0,-1)];
  float4 pzp = VelocityGrid0[index + int3( 0, 0, 1)];
  Divergence_RW[index] = (pxp.x-pxm.x + pyp.y-pym.y + pzp.z-pzm.z) * 0.5;
}

// Divergence, Pressure -> Pressure'
[numthreads(16, 4, 4)]
void JacobiCS(uint3 i : SV_DispatchThreadID)
{
  float divergence = Divergence[i];
  float pxm = Pressure[i + int3(-1, 0, 0)];
  float pxp = Pressure[i + int3( 1, 0, 0)];
  float pym = Pressure[i + int3( 0,-1, 0)];
  float pyp = Pressure[i + int3( 0, 1, 0)];
  float pzm = Pressure[i + int3( 0, 0,-1)];
  float pzp = Pressure[i + int3( 0, 0, 1)];
  Pressure_RW[i] = (pxp+pxm+pyp+pym+pzp+pzm - divergence) / 6;
}

// VelocityGrid1, Pressure -> VelocityGrid0
[numthreads(16, 4, 4)]
void ProjectCS(uint3 i : SV_DispatchThreadID)
{
  float pxm = Pressure[i + int3(-1, 0, 0)];
  float pxp = Pressure[i + int3( 1, 0, 0)];
  float pym = Pressure[i + int3( 0,-1, 0)];
  float pyp = Pressure[i + int3( 0, 1, 0)];
  float pzm = Pressure[i + int3( 0, 0,-1)];
  float pzp = Pressure[i + int3( 0, 0, 1)];

  float3 grad = float3(pxp-pxm, pyp-pym, pzp-pzm) * 0.5;
  float3 vel  = VelocityGrid1[i].xyz - grad;
#if 0
  if (UseVorticityConfinement) {
    float3 vf = vorticity_confinement(i);
    vel += (vf * 0.1);
  }
#endif
  VelocityGrid0_RW[i] = float4(vel, 0);
}

[numthreads(16, 4, 4)]
void AddPressureCS(uint3 id : SV_DispatchThreadID)
{
	float3 worldPos = ((float3)id + 0.5) * DimensionInv * GridSize + cGridPositionMin.xyz;
	for (int i = 0; i < cPressureSourceCount; ++i)
	{
		PressureSource ps = gPressureSources[i];
		float3 v = ps.position - worldPos;
		if (dot(v, v) < ps.radius*ps.radius) {
			float power = exp(-dot(v, v)) * ps.pressure;
			Pressure_RW[id] = power;
		}
	}
}

[numthreads(16, 4, 4)]
void ResetCS(uint3 id : SV_DispatchThreadID)
{
	VelocityGrid0_RW[id] = float4(0, 0, 0, 0);
	Pressure_RW[id] = 0;
}

// VelocityGrid0 -> RotationGrid
[numthreads(16, 4, 4)]
void RotationVelocityCS(uint3 i : SV_DispatchThreadID)
{
  float3 xm = VelocityGrid0[i + int3(-1, 0, 0)].xyz;
  float3 xp = VelocityGrid0[i + int3( 1, 0, 0)].xyz;
  float3 ym = VelocityGrid0[i + int3( 0,-1, 0)].xyz;
  float3 yp = VelocityGrid0[i + int3( 0, 1, 0)].xyz;
  float3 zm = VelocityGrid0[i + int3( 0, 0,-1)].xyz;
  float3 zp = VelocityGrid0[i + int3( 0, 0, 1)].xyz;
  float ryz = (yp.z - ym.z) * 0.5;
  float rzy = (zp.y - zm.y) * 0.5;
  float rzx = (zp.x - zm.x) * 0.5;
  float rxz = (xp.z - xm.z) * 0.5;
  float rxy = (xp.y - xm.y) * 0.5;
  float ryx = (yp.x - ym.x) * 0.5;
  float3 ru = { ryz-rzy, rzx-rxz, rxy-ryx };
  RotationGrid_RW[i] = float4(ru, length(ru));
}

float3 vorticity_confinement(uint3 i)
{
  float4 rot = RotationGrid[i];
  if (rot.w < 0.0001) return float3(0, 0, 0);
  float4 xm = RotationGrid[i + int3(-1, 0, 0)];
  float4 xp = RotationGrid[i + int3( 1, 0, 0)];
  float4 ym = RotationGrid[i + int3( 0,-1, 0)];
  float4 yp = RotationGrid[i + int3( 0, 1, 0)];
  float4 zm = RotationGrid[i + int3( 0, 0,-1)];
  float4 zp = RotationGrid[i + int3( 0, 0, 1)];
  float3 grad_w = float3(xp.w-xm.w, yp.w-ym.w, zp.w-zm.w) * 0.5;
  if (length(grad_w) < 0.0001) return float3(0, 0, 0);
  float3 n  = normalize(grad_w);
  return cross(n, rot.xyz);
}

#if 0
// ============================================================================
//
//  obstacles
//
// ============================================================================
struct VS_ObstacleVelocityOutput
{
  float3 position;
  float3 velocity;
};

struct GS_ObstacleVelocityOutput
{

};

struct IntersectionPoint
{
  float2 position;
  float3 velocity;
};

bool check_intersection(VS_ObstacleVelocityOutput v1,
                        VS_ObstacleVelocityOutput v2,
                        float                     slice_z,
                        inout IntersectionPoint   intersection)
{
  float t = (slice_z - v1.position.z) / (v2.position.z - v1.position.z);
  if (t < 0 || 1 < t) return false;
  intersection.position = lerp(v1.position, v2.position, t).xy;
  intersection.velocity = lerp(v1.velocity, v2.velocity, t);
}

[maxvertexcount(3)]
void GS_ObstacleVelocity(triangle VS_ObstacleVelocityOutput input[3],
                         inout TriangleStream<GS_ObstacleVelocityOutput> stream)
{
  float min_z = min(min(input[0].position.z, input[1].position.z), input[2].position.z);
  float max_z = max(max(input[0].position.z, input[1].position.z), input[2].position.z);
  if (SliceZ < min_z || max_z < SlizeZ) return;

  int index = 0;
  IntersectionPoint intersections[2];
  if (check_intersection(input[0], input[1], SliceZ, intersection[index])) {
    index += 1;
  }
  if (check_intersection(input[1], input[2], SliceZ, intersection[index])) {
    index += 1;
  }
  if (index < 2 && check_intersection(input[2], input[0], SliceZ, intersection[index])) {
    index += 1;
  }
  if (index < 2) return;

  GS_ObstacleVelocityOutput output;
  for (int i=0; i < 2; ++i) {
//    output.position = float4(intersections[i].position + (normal*
    output.velocity = intersections[i].velocity;
  }

  stream.RestartStrip();
}

#endif

// ============================================================================
//
//  render functions
//
// ============================================================================
struct VS_RenderVolumeInput
{
  float3 position : POSITION;
};

struct VS_RenderVolumeOutput
{
  float4 position : SV_Position;
};

struct PS_OutputColor
{
  float4  color : SV_Target;
};

VS_RenderVolumeOutput RenderVolumeVS(VS_RenderVolumeInput input)
{
  VS_RenderVolumeOutput output;
  output.position = mul(mul(float4(input.position, 1), WorldMatrix), cViewProjectionMatrix);
  return output;
}

PS_OutputColor RenderVolumePS(VS_RenderVolumeOutput input)
{
  // スクリーン座標からレイを作成してワールド座標へ
  float4 ray_direction = float4( ((((input.position.x/* - ScreenOffset.x*/) * 2) / cResolution.x) -1) / cProjectionMatrix[0][0],
                                -((((input.position.y/* - ScreenOffset.y*/) * 2) / cResolution.y) -1) / cProjectionMatrix[1][1],
                                1, 0);
  ray_direction = normalize(mul(ray_direction, cViewInverseMatrix));
  float4 ray_origin = float4(cViewInverseMatrix[3][0], cViewInverseMatrix[3][1], cViewInverseMatrix[3][2], 1);

  // シミュレーションボリュームの平面6枚との交差判定
  float3 t1 = max((-1 - ray_origin) / ray_direction, 0).xyz;
  float3 t2 = max(( 1 - ray_origin) / ray_direction, 0).xyz;

  float3 front = min(t1, t2);
  float3 back  = max(t1, t2);

  float tfront = max(front.x, max(front.y, front.z));
  float tback  = min(back.x, min(back.y, back.z));

  float3 texf = ((ray_origin + tfront*ray_direction + 1)/2).xyz;
  float3 texb = ((ray_origin + tback *ray_direction + 1)/2).xyz;

  float steps = floor(length(texf - texb) * Dimension.x+0.5);
  float3 texdir = (texb-texf)/steps;
  steps = (tfront >= tback)? 0: steps;

  float3 max_sample = 0;
  float4 max_s = 0;
  float  m = 0;
  for (float i=0.5; i < steps; ++i) {
    float3 sample = texf + i*texdir;
    float4 s = VelocityGrid0.SampleLevel(gSamplerBilinearClamp, sample, 0);
    max_sample = max(sample, max_sample);
    max_s      = max(s, max_s);
    m          = max(m, length(s.xyz));
  }

  PS_OutputColor output;
  output.color = saturate(lerp(float4(0,-1.41,-3,-0.4), float4(1.41,1.41,1,1.41),m/3));//saturate(lerp(float4(0,-1.41,-3, -0.4), float4(1.41,1.41,1,1.41), m/3));
                 //VelocityGrid0.Sample(LinearSampler, float3(0.5,0.5,0));;
                 //VelocityGrid1.Sample(LinearSampler, float3(0.5,0.5,0));;
                 //float4(abs(tfront - tback) / 4,
                //0, 0, 1);
  return output;
}

float4 RenderGridPS(VS_RenderVolumeOutput input) : SV_Target
{
	return float4(1,0,0,1);
}

struct VS_RenderVolumeSliceInput
{
  float3 position : POSITION;
  float3 texcoord : TEXCOORD;
};

struct VS_RenderVolumeSliceOutput
{
  float4 position : SV_Position;
  float3 texcoord : TEXCOORD;
};

VS_RenderVolumeSliceOutput VS_RenderVolumeSlice(VS_RenderVolumeSliceInput input)
{
  VS_RenderVolumeSliceOutput output;
  output.position = mul(float4(input.position, 1), WorldMatrix * cViewProjectionMatrix);
  output.texcoord = input.texcoord;
  return output;
}

PS_OutputColor PS_RenderVolumeSlice(VS_RenderVolumeSliceOutput input)
{
  PS_OutputColor output;
  output.color = VelocityGrid0.Sample(gSamplerNearestClamp, input.texcoord);
  return output;
}
