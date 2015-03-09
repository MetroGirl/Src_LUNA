#pragma once
#include "app/renderPass.h"
#include "app/textureResource.h"
#include "app/renderPass/context.h"
#include <DirectXMath.h>
#include <functional>

namespace luna {
using namespace DirectX;

struct ParticleEmitter
{
	XMFLOAT3 mPosition;
	f32 mRadius;
	XMFLOAT4 mColor;
	u32 mEmitCount;
	f32 mEmitLifeTime;
	u32 mUniversalIndex;
	f32 _;
};

struct ParticleEmitterSettings
{
	ParticleEmitter mEmitter;
	std::string mShaderName;
};

class ParticleSimulationPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(ParticleSimulationPass, RenderPass);
public:
	explicit ParticleSimulationPass(RenderPassContext& rpc);

	void addEmitterController(std::function<void (ParticleEmitterSettings&)> const& f)
	{
		mEmitterControllers.push_back(f);
	}

	void setShader(std::string const& shaderName)
	{
		mShaderName = shaderName;
	}

private:
	void preSettings(RenderPassContext& rpc) override;
	void postSettings(RenderPassContext& rpc) override;
	void render(RenderPassContext& rpc) const override;

	std::vector<std::function<void (ParticleEmitterSettings&)>> mEmitterControllers;
	Resource<ID3D11Buffer> mEmitterRandomSeed;
	ComPtr<ID3D11Buffer> mEmitterCB;
	ComPtr<ID3D11Buffer> mSortCB;
	Resource<ID3D11Buffer> mSortTmp;
	ComPtr<ID3D11Buffer> mFluidVelocityCB;
	u32 const mMaxParticleCount;
	u32 const mMaxEmitParticleCount;
	std::string mShaderName;
	mutable bool mFirstTime{true};
};

class ParticleShadowPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(ParticleShadowPass, RenderPass);
public:
	explicit ParticleShadowPass(RenderPassContext& rpc);
	~ParticleShadowPass();

	void preSettings(RenderPassContext& rpc) override;
	void postSettings(RenderPassContext& rpc) override;
	void render(RenderPassContext& rpc) const override;

	ComPtr<ID3D11Buffer> mCB;
	ComPtr<ID3D11RasterizerState> mRS;
};

class ParticleRenderPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(ParticleRenderPass, RenderPass);
public:
	explicit ParticleRenderPass(RenderPassContext& rpc);
	~ParticleRenderPass();

	void setShader(std::string const& shaderName)
	{
		mShaderName = shaderName;
	}

private:
	void preSettings(RenderPassContext& rpc) override;
	void postSettings(RenderPassContext& rpc) override;
	void render(RenderPassContext& rpc) const override;

	std::string mShaderName;
	TextureResource* mParticleTexture;
	ComPtr<ID3D11BlendState> mBlendState;
	ComPtr<ID3D11DepthStencilState> mDepthStencilState;
	ComPtr<ID3D11SamplerState> mShadowSampler;
};

}
