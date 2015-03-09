#ifndef COMMON_HLSLI
#define COMMON_HLSLI

cbuffer SceneCB : register(b0)
{
	float4x4 cViewMatrix;
	float4x4 cProjectionMatrix;
	float4x4 cViewProjectionMatrix;
	float4x4 cViewInverseMatrix;
	float4 cEyePosition;
	float2 cResolution;
	float2 cResolutionInv;
	float cDemoTimeSecond;//!< デモ開始からの経過秒数
	float cDemoTimeRatio;//!< デモ全体の進行度(0.0～1.0)
	float cAspectV;//!< プロジェクタ用に色調補正を行うか否か
	float cAspectH;//!< モニタアスペクト比(シーンは16:9で作られているので補正する)
	float4 cSceneTimeSecond[4];//!< 現在のシーン開始からの経過秒数
	float4 cSceneTimeRatio[4];//!< 現在のシーン進行度(0.0～1.0)
	float4 cSoundFFTBand[36];//!< バンドに分割されたFFT Magnitude
	float4 cSoundFFTAvg;
	float4 cSettings;// x:bigScreen?
}

SamplerState gSamplerNearestClamp : register(s4);
SamplerState gSamplerNearestRepeat : register(s5);
SamplerState gSamplerNearestMirror : register(s6);
SamplerState gSamplerBilinearClamp : register(s7);
SamplerState gSamplerBilinearRepeat : register(s8);
SamplerState gSamplerBilinearMirror : register(s9);
SamplerState gSamplerTrilinearClamp : register(s10);
SamplerState gSamplerTrilinearRepeat : register(s11);
SamplerState gSamplerTrilinearMirror : register(s12);
SamplerState gSamplerAnisotropicClamp : register(s13);
SamplerState gSamplerAnisotropicRepeat : register(s14);
SamplerState gSamplerAnisotropicMirror : register(s15);


float4 RGBtoHSV(float4 rgb)
{
	const float r = rgb.x;
	const float g = rgb.y;
	const float b = rgb.z;

	float max = r > g ? r : g;
	max = max > b ? max : b;
	float min = r < g ? r : g;
	min = min < b ? min : b;
	float h = max - min;
	if (h > 0.0f) {
		if (max == r) {
			h = (g - b) / h;
			if (h < 0.0f) {
				h += 6.0f;
			}
		}
		else if (max == g) {
			h = 2.0f + (b - r) / h;
		}
		else {
			h = 4.0f + (r - g) / h;
		}
	}
	h /= 6.0f;
	float s = (max - min);
	if (max != 0.0f)
		s /= max;
	float v = max;

	return float4(h, s, v, rgb.w);
}

float4 HSVtoRGB(float4 hsv)
{
	const float h = hsv.x * 6.f;
	const float s = hsv.y;
	const float v = hsv.z;

	float r = v;
	float g = v;
	float b = v;
	if (s > 0.0f) {
		const int i = (int)h;
		const float f = h - (float)i;
		switch (i) {
		default:
		case 0:
			g *= 1 - s * (1 - f);
			b *= 1 - s;
			break;
		case 1:
			r *= 1 - s * f;
			b *= 1 - s;
			break;
		case 2:
			r *= 1 - s;
			b *= 1 - s * (1 - f);
			break;
		case 3:
			r *= 1 - s;
			g *= 1 - s * f;
			break;
		case 4:
			r *= 1 - s * (1 - f);
			g *= 1 - s;
			break;
		case 5:
			g *= 1 - s;
			b *= 1 - s * f;
			break;
		}
	}

	return float4(r, g, b, hsv.w);
}


float4 BlendOpCore(float4 src, float4 dst, float4 sfactor, float4 dfactor)
{
	return float4(src.r*sfactor.r + dst.r*dfactor.r, src.g*sfactor.g + dst.g*dfactor.g, src.b*sfactor.b + dst.b*dfactor.b, src.a*sfactor.a + dst.a*dfactor.a);
}

float4 BlendOpOne(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(1,1,1,1), float4(0,0,0,0));
}

float4 BlendOpAdd(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(src.a,src.a,src.a,src.a), float4(1,1,1,1));
}

float4 BlendOpMul(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(0,0,0,0), float4(src.rgb, 1));
}

float4 BlendOpInv(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(1-dst.r,1-dst.g,1-dst.b,1-dst.a), float4(0,0,0,0));
}

float4 BlendOpScreen(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(1-dst.r,1-dst.g,1-dst.b,1-dst.a), float4(1,1,1,1));
}

float4 BlendOpLogicalOR(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(1-dst.r,1-dst.g,1-dst.b,1-dst.a), float4(1-src.r,1-src.g,1-src.b,1-src.a));
}

float4 BlendOpAlpha(float4 src, float4 dst)
{
	return BlendOpCore(src, dst, float4(src.a,src.a,src.a,src.a), float4(1-src.a,1-src.a,1-src.a,1-src.a));
}

float hash( float2 p ) {
	float h = dot(p,float2(127.1,311.7));	
	return frac(sin(h)*43758.5453123);
}
float noise( float2 p ) {
	float2 i = floor( p );
	float2 f = frac( p );	
	float2 u = f*f*(3.0-2.0*f);
	return -1.0+2.0*lerp( lerp( hash( i + float2(0.0,0.0) ), hash( i + float2(1.0,0.0) ), u.x), lerp( hash( i + float2(0.0,1.0) ), hash( i + float2(1.0,1.0) ), u.x), u.y);
}

uint4 xorshift(uint4 seed)
{
  uint t = seed.x ^ (seed.x << 11);
  seed.x = seed.y;
  seed.y = seed.z;
  seed.z = seed.w;
  seed.w = (seed.w ^ (seed.w >> 19)) ^ (t ^ (t >> 8));
  return seed;
}

float random(uint value)
{
  // 0 ~ 1
  return clamp(((float)value / 0xffffffff), 0, 1);
}


struct Vertex
{
	float3 position;
	float3 normal;
	float2 uv;
};


#endif
