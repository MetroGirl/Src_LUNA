#include "common.hlsli"	
#include "fullScreenTriangle.hlsl"

float mod(float x, float y)
{
  return x - y * floor(x/y);
}

// ---------------------------------------------
Texture2D<float4> srcCBuf0 : register( t0 );
Texture2D<float4> srcCBuf1 : register( t1 );
Texture2D<float4> srcCBuf2 : register( t2 );
Texture2D<float4> srcCBuf3 : register( t3 );
Texture2D<float4> srcZBuf0 : register( t4 );
Texture2D<float4> srcZBuf1 : register( t5 );
Texture2D<float4> srcZBuf2 : register( t6 );
Texture2D<float4> srcZBuf3 : register( t7 );
Texture2D<float4> srcScene : register( t0 );

Texture2D<float4> tOverlay : register( t13 );

float4 tex2D(Texture2D<float4> texObj, float2 samplePos)
{
	return texObj.Sample(gSamplerBilinearClamp, samplePos);
}

// ---------------------------------------------
cbuffer PostEffect : register( b2 )
{
	float4 peSceneRatio;
	float4 peEffectRatio;
	float4 peMetadata1;
	float4 peMetadata2;
}

// ---------------------------------------------

float4 WhitePostEffectPS(FullScreenTriangleVSOut input) : SV_Target0
{
  return float4(1,1,1,1);
}

// ---------------------------------------------

float4 CopyAsIsPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float4 sceneColor = srcScene.Load(int3(input.Position.x, input.Position.y, 0));
	return float4(sceneColor.xyz, 1.0);
}

// ---------------------------------------------

float4 FinalCombinerPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;

	const float aspectOriginW = 16.f / 9.f;
	const float aspectOriginH = 9.f / 16.f;
	const float aspectW = cAspectV / cAspectH;
	const float aspectH = cAspectH / cAspectV;
	const float renderMarginW = 1.f - aspectOriginW*aspectW;
	const float renderMarginH = 1.f - aspectOriginH*aspectH;
	const float renderMarginUnitW = max(renderMarginW / 2.f, 0.f);
	const float renderMarginUnitH = max(renderMarginH / 2.f, 0.f);

	float4 sceneColor;
	const float4 edgeColor = float4(0,0,0,1);

	if( uv.y<=renderMarginUnitH ){
		sceneColor = edgeColor;
	}
	else if( uv.y>=1.f-renderMarginUnitH ){
		sceneColor = edgeColor;
	}
	else if( uv.x<=renderMarginUnitW ){
		sceneColor = edgeColor;
	}
	else if( uv.x>=1.f-renderMarginUnitW ){
		sceneColor = edgeColor;
	}
	else{
		uv.x -= renderMarginUnitW;
		uv.x /= 1.0 - renderMarginUnitW*2;
		uv.y -= renderMarginUnitH;
		uv.y /= 1.0 - renderMarginUnitH*2;
		sceneColor = tex2D(srcScene, uv);
	}

	// fake gamma for big screen
	if( cSettings.x > 0.f ){
		sceneColor = pow(sceneColor, 2.2f);
	}

#if 0//FFT Sample
	float mag = cSoundFFTBand[10];
	if( mag<0.5f ){
		mag = 1;
	}
	sceneColor *= mag;
#endif

	return float4(sceneColor.xyz, 1.0);
}

// --------------------------------------------

#define SAMPLE_NUM 16
#define BLUR_START 1.0f
#define BLUR_WIDTH 0.1f
float4 RadialBlurPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 samplePos = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);
	
//	return float4(srcColor.xyz, 1);

	float sceneRatio = peEffectRatio.x;
	float BlurWidth = 0.001;
#if 1
	if( sceneRatio<0.5f ){
		BlurWidth = BLUR_WIDTH*(sceneRatio*2.f);
	}else{
		BlurWidth = BLUR_WIDTH*(1.f-(sceneRatio-0.5f)*2.0f);
	}
#endif

	float4 tmpColor = float4(0,0,0,0);
	float2 Center = float2(0.5f,0.5f);
	samplePos -= Center;
	for(int i=0; i<SAMPLE_NUM; i++) {
		float scale = BLUR_START - BlurWidth*(i/(((float)SAMPLE_NUM)-1.0f));
		float4 texColor = srcScene.Sample(gSamplerNearestClamp, samplePos*scale + Center );
		tmpColor += texColor;
	}
	tmpColor /= (float)SAMPLE_NUM;
	return float4(BlendOpAlpha(float4(tmpColor.xyz, peEffectRatio.x), tOverlay.Sample(gSamplerBilinearClamp, input.TexCoord)).xyz, 1);
}

float4 NoisePS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 samplePos = input.TexCoord;
//	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);
//	return float4(srcColor.xyz, 1);

	float rate = min(peEffectRatio.x * 4.0f, 1);
	float brate = 1;//min(max(0, 9.0f - peEffectRatio.x * 10.0f), 1);
	float randValue = noise(samplePos);
	return float4(
			tex2D(srcScene, samplePos + rate * 0.1f * randValue).r,
			tex2D(srcScene, samplePos + rate * 0.15f * randValue).g,
			tex2D(srcScene, samplePos + rate * 0.30f * randValue).b,
			1
			)
	 * brate;
}

// ---------------------------------------------

float4 ChromaticAberrationPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, uv);

	float centerBuffer 		= 0.15;
	float vignetteStrength 	= 1.5;
	float aberrationStrength 	= 0.25;
	
	float chrDist;
	float vigDist;
	
	float2 vecDist = uv - float2( 0.5 , 0.5 );
	chrDist = vigDist = length( vecDist );
	
	chrDist	-= centerBuffer;
	if( chrDist < 0.0 ) chrDist = 0.0;
	
	float2 uvR = uv * ( 1.0 + chrDist * 0.02 * aberrationStrength ),
		 uvB = uv * ( 1.0 - chrDist * 0.02 * aberrationStrength );
	
	float4 c;
	c.x = tex2D( srcScene , uvR ).x; 
	c.y = tex2D( srcScene , uv ).y; 
	c.z = tex2D( srcScene , uvB ).z;
	// c *= 1.0 - vigDist* vignetteStrength;

	return float4(c.xyz, 1);
}

// ---------------------------------------------

float4 VignettingPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;

	float OuterVig = 0.97;
	float InnerVig = 0.33;

	float4 color = tex2D(srcScene, uv);
	float2 center = float2(0.5,0.5);

	float dist  = distance(center,uv )*1.414213;
#if 1
	float vig = 1-pow(dist, 0.8);
#else
	float vig = clamp((OuterVig-dist) / (OuterVig-InnerVig),0.0,1.0);
#endif
	vig *= 2.f;
	
	color *= vig;

	return float4(color.xyz, 1);
}

// ---------------------------------------------

float4 FakeGammaPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;
	float4 color = tex2D(srcScene, uv);

	color = pow(color, 2.2);

	return float4(color.xyz, 1);
}

// ---------------------------------------------

float4 BigScreenGammaPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;
	float4 color = tex2D(srcScene, uv);

	color = pow(color, 2.2);

	return float4(color.xyz, 1);
}

// ---------------------------------------------

float4 FilmGrainPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;
	float4 srcColor = tex2D(srcScene, uv);
	float strength = 32.0;

	float x = (uv.x + 4.0) * (uv.y + 4.0) * ((peEffectRatio.x+0.01f) * 1000.0);
	float g = mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005;
	float4 grain = float4(g,g,g,g) * strength;

	float4 c;
	grain = 1.0 - grain;
	c = srcColor * grain;
	//c = srcColor + grain;

	return float4(c.xyz, 1);
}


// ---------------------------------------------

cbuffer Loader : register( b1 )
{
	float4 loaderProgress;
	float loaderAspectV;
	float loaderAspectH;
	float loaderPadding_;
	float loaderPadding__;
}

Texture2D<float4> loaderScreen : register( t0 );

float4 LoaderPS(FullScreenTriangleVSOut input) : SV_Target0
{
	const float NEON_WIDTH = 0.01;
	float2 uv = input.TexCoord;

	const float aspectOriginW = 16.f / 9.f;
	const float aspectOriginH = 9.f / 16.f;
	const float aspectW = loaderAspectV / loaderAspectH;
	const float aspectH = loaderAspectH / loaderAspectV;
	const float renderMarginW = 1.f - aspectOriginW*aspectW;
	const float renderMarginH = 1.f - aspectOriginH*aspectH;
	const float renderMarginUnitW = max(renderMarginW / 2.f, 0.f);
	const float renderMarginUnitH = max(renderMarginH / 2.f, 0.f);

	float4 color;
	const float4 edgeColor = float4(0,0,0,1);

	if( uv.y<=renderMarginUnitH ){
		color = edgeColor;
	}
	else if( uv.y>=1.f-renderMarginUnitH ){
		color = edgeColor;
	}
	else if( uv.x<=renderMarginUnitW ){
		color = edgeColor;
	}
	else if( uv.x>=1.f-renderMarginUnitW ){
		color = edgeColor;
	}
	else{
		uv.x -= renderMarginUnitW;
		uv.x /= 1.0 - renderMarginUnitW*2;
		uv.y -= renderMarginUnitH;
		uv.y /= 1.0 - renderMarginUnitH*2;

	 	color = tex2D(loaderScreen, uv);

	  float h = 0.7;
	 
	  float t = abs(uv.y - h) / NEON_WIDTH;
	  float c = 1.0 - t;
	 
	  if(c > 0.0 && uv.x>loaderProgress.x)
	  {
	    c = pow(c, 3.0);
	    float3 rc = float3(c, c, c);
	    color += float4(rc, 1);
	  }

		color *= 1-loaderProgress.x;
	}
	return float4(color.xyz, 1);
}

// ---------------------------------------------

cbuffer ExitFade : register( b1 )
{
	float4 exitFadeColor;
}

float4 ExitFadePS(FullScreenTriangleVSOut input) : SV_Target0
{
	return exitFadeColor;
}

// ----------------------------------------------
Texture2D<float4> debugTargetTexture : register( t1 );

float4 DebugDispCapturePS(FullScreenTriangleVSOut input) : SV_Target0
{
	return float4(debugTargetTexture.Load(int3(input.Position.x, input.Position.y, 0)).xyz, 1);
}

// ----------------------------------------------
float4 ColorBleedingPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 samplePos = input.TexCoord;

	float randValue = (noise(samplePos + peEffectRatio.x) + 1.0) / 2.0;

	float rate = sin(peEffectRatio.x * 3.1415);

	float2 rrs = samplePos;
	float2 rgs = samplePos;
	float2 rbs = samplePos;

	rrs.x += 0.006;
	rgs.x += 0.003;
	rbs.x += 0.009;

	rrs = lerp(samplePos, rrs, rate);
	rgs = lerp(samplePos, rgs, rate);
	rbs = lerp(samplePos, rbs, rate);

	float2 lrs = samplePos;
	float2 lgs = samplePos;
	float2 lbs = samplePos;

	lrs.x -= 0.006;
	lgs.x -= 0.003;
	lbs.x -= 0.009;

	lrs = lerp(samplePos, lrs, rate);
	lgs = lerp(samplePos, lgs, rate);
	lbs = lerp(samplePos, lbs, rate);

	float4 ri = float4(
			srcScene.Sample(gSamplerBilinearRepeat, rrs).r,
			srcScene.Sample(gSamplerBilinearRepeat, rgs).g,
			srcScene.Sample(gSamplerBilinearRepeat, rbs).b,
			1);
	float4 li = float4(
			srcScene.Sample(gSamplerBilinearRepeat, lrs).r,
			srcScene.Sample(gSamplerBilinearRepeat, lgs).g,
			srcScene.Sample(gSamplerBilinearRepeat, lbs).b,
			1);

	return float4(BlendOpAlpha(float4(ri.xyz, 0.5), li).xyz, 1);
}

float4 pickGreetzTexture(float2 uv, float4 rect)
{
	float boxWidthInPixel = 600;
	float lineHeightInPixel = 240;
	float textureWidthInPixel = 4096;
	float numColumn = 5;
	float uvWidthPerBox = boxWidthInPixel / textureWidthInPixel;
	float uvHeightPerLine = lineHeightInPixel / textureWidthInPixel;

	uv.x *= uvWidthPerBox;
	// uv.y *= uvHeightPerLine;


	float4 tex = float4(tex2D(tOverlay, uv).xyz, 1);

	uv.x *= 4096;
	uv.y *= 1024;

	if(uv.x > rect.x+rect.z){
		tex.a = 0;
	}
	if(uv.y > rect.y+rect.w){
		tex.a = 0;
	}

	uv.x /= 4096;
	uv.y /= 1024;

	return tex;
}

float4 SampleGreetz(float2 uv, float2 dispPos, float2 dispSize, float2 samplePos, float2 sampleSize)
{
	if( uv.x<dispPos.x || uv.x>=dispPos.x+dispSize.x ){
		return float4(-1,-1,-1,-1);
	}
	if( uv.y<dispPos.y || uv.y>=dispPos.y+dispSize.y ){
		return float4(-1,-1,-1,-1);
	}

	float2 tuv = uv;
	tuv.x -= dispPos.x-samplePos.x;
	tuv.y -= dispPos.y-samplePos.y;

	float4 c = tex2D(tOverlay, tuv);
	if( length(c.xyz)==0 ){
		return float4(-1,-1,-1,-1);
	}
	return c;
}

// ----------------------------------------------
float4 GreetzPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;
	//return float4(tex2D(tOverlay, uv).xyz, 1);
	
	//float4 tex = pickGreetzTexture(uv, float4(0,0,600,80));
	float ratio = peEffectRatio.x;
	float invRatio = 1 - ratio;
	//invRatio /= 2;

	float appearValue = 1*ratio*2;
	float vanishValue = ratio>0.5f ? 2*(ratio-0.5) : 0;
	
	float2 dispPos;
	float2 dispSize;// = float2(0.315, appearValue);
	float2 samplePos;
	float2 sampleSize = float2(0.2,0.2);
	float4 gc;
	{
		dispSize = float2(0.31, appearValue);
		dispPos = float2(0.05, vanishValue);
		samplePos = float2(0.0, vanishValue);
		gc = SampleGreetz(uv, dispPos, dispSize, samplePos, sampleSize);
		if( gc.x<0 ){
#if 1
			dispSize = float2(0.315, 1);
			dispPos = float2(0.35, 1-ratio*2);
			samplePos = float2(0.3, 1-ratio*2);
			gc = SampleGreetz(uv, dispPos, dispSize, samplePos, sampleSize);
			if( gc.x<0 ){
				dispSize = float2(0.287, appearValue);
				dispPos = float2(0.7, vanishValue);
				samplePos = float2(0.614, vanishValue);
	gc = SampleGreetz(uv, dispPos, dispSize, samplePos, sampleSize);
				if( gc.x<0 ){
					return float4(tex2D(srcScene, uv).xyz, 1);
				}
			}
#else
			return float4(tex2D(srcScene, uv).xyz, 1);
#endif
		}
	}
	
	gc *= 3;

	gc = BlendOpAlpha(float4(gc.xyz, 0.8), float4(tex2D(srcScene, uv).xyz, 1));

	return float4(gc.xyz, 1);

	return float4(0,1,0,1);



	uv.x *= 4096;
	uv.y *= 1024;


	uv.x /= 4096;
	uv.y /= 1024;

//	return tex;
//	return float4(tOverlay.Load(int3(uv.x, uv.y, 0)).xyz, 1);
}

// ----------------------------------------------
float4 GreetzIntroPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 uv = input.TexCoord;
	float ratio = peEffectRatio.x;
	float invRatio = 1 - ratio;

	float appearValue = 1*ratio;
	float vanishValue = 0;//ratio>0.5f ? 2*(ratio-0.5) : 0;
	
	float2 dispPos;
	float2 dispSize;
	float2 samplePos;
	float2 sampleSize = float2(0.2,0.2);
	float4 gc;
	{
		dispSize = float2(0.31, appearValue);
		dispPos = float2(0.465, vanishValue);
		samplePos = float2(0.898, vanishValue);
		gc = SampleGreetz(uv, dispPos, dispSize, samplePos, sampleSize);
		if( gc.x<0 ){
			return float4(tex2D(srcScene, uv).xyz, 1);
		}
	}
	
	gc *= 3;

	float alpha = 0.8;
	if( ratio>0.5 ){
		alpha = 1 - (2*(ratio-0.5));
	}
	gc = BlendOpAlpha(float4(gc.xyz, alpha), float4(tex2D(srcScene, uv).xyz, 1));

	return float4(gc.xyz, 1);

	return float4(0,1,0,1);



	uv.x *= 4096;
	uv.y *= 1024;


	uv.x /= 4096;
	uv.y /= 1024;

//	return tex;
//	return float4(tOverlay.Load(int3(uv.x, uv.y, 0)).xyz, 1);
}

float4 IntroScreenPS(FullScreenTriangleVSOut input) : SV_Target0
{
#if 0
	float2 samplePos = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);

	float sceneRatio = peEffectRatio.x;
	float BlurWidth = 0.001;

	float rate = peEffectRatio.x*peEffectRatio.x;

	float3 ccc = tOverlay.Sample(gSamplerBilinearClamp, input.TexCoord).xyz;
	if( length(ccc)==0 ){
		ccc = srcColor;
	}

	return float4(BlendOpAlpha(float4(ccc, 1-rate), srcColor).xyz, 1);
#else
	float2 samplePos = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);

	float sceneRatio = peEffectRatio.x;
	float BlurWidth = 0.001;

	float rate = peEffectRatio.x<0.5 ? 1-peEffectRatio.x : peEffectRatio.x;

	float3 ccc = tOverlay.Sample(gSamplerBilinearClamp, input.TexCoord).xyz;
	if( length(ccc)==0 ){
		ccc = srcColor;
	}

	return float4(BlendOpAlpha(float4(srcColor.xyz, rate), float4(ccc.xyz, 1)).xyz, 1);
#endif
}

float4 HelloScreenPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 samplePos = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);

	float sceneRatio = peEffectRatio.x;
	float BlurWidth = 0.001;

	float rate = peEffectRatio.x*peEffectRatio.x;

	return float4(BlendOpAlpha(float4(tOverlay.Sample(gSamplerBilinearClamp, input.TexCoord).xyz, 1-rate), srcColor).xyz, 1);
}

float4 OutroScreenPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 samplePos = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);

	float sceneRatio = peEffectRatio.x;
	float BlurWidth = 0.001;

	float rate = peEffectRatio.x<0.5 ? 1-peEffectRatio.x : peEffectRatio.x;

	float4 tmpColor = float4(0,0,0,0);
	float2 Center = float2(0.5f,0.5f);
	samplePos -= Center;
	for(int i=0; i<1; i++) {
		float scale = BLUR_START - BlurWidth*(i/(((float)SAMPLE_NUM)-1.0f));
		float4 texColor = srcScene.Sample(gSamplerNearestClamp, samplePos*scale + Center );
		tmpColor += texColor;
	}
	tmpColor /= (float)SAMPLE_NUM;
	return float4(BlendOpAlpha(float4(srcColor.xyz, rate), tOverlay.Sample(gSamplerBilinearClamp, input.TexCoord)).xyz, 1);
}


float4 ColorCorrectionPS(FullScreenTriangleVSOut input) : SV_Target0
{
	float2 samplePos = input.TexCoord;
	float4 srcColor = srcScene.Sample(gSamplerNearestClamp, samplePos);
	return srcColor;

	float4 finalColor = float4(srcColor.xyz, 1);

	float4 c = RGBtoHSV(finalColor);
	c.g *= (cSoundFFTBand[3]+cSoundFFTBand[0]+cSoundFFTBand[2]) / 3;
	//c.g *= cSoundFFTBand[3];
	c = HSVtoRGB(c);

	finalColor = c;

	float mag = cSoundFFTBand[0];
	if( mag<0.5f ){
		//mag = 1;
	}
	// finalColor *= mag;

	return finalColor;
}


