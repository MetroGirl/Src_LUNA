#pragma once
#include "lib/singleton.h"
#include "lib/gfx/frameBuffer.h"
#include "lib/gfx/samplerState.h"
#include "lib/gfx/blendState.h"
#include "app/renderPass.h"
#include "app/renderPass/context.h"
#include <unordered_map>
#include <functional>

namespace luna {

// 名前は Solid だけど priority ソートするので万能描画パスになります
class ObjectPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(ObjectPass, RenderPass);
public:
	ObjectPass(RenderPassContext& rpc) {}

	template <class Draw>
	void draw(u32 priority, Draw const& draw)
	{
		mDrawList.push_back(std::make_pair(priority, draw));
	}

private:
	void preSettings(RenderPassContext& rpc) override
	{
		std::stable_sort(begin(mDrawList), end(mDrawList),
			[](std::pair<u32, std::function<void(RenderPassContext&)>> const& lhs,
			std::pair<u32, std::function<void(RenderPassContext&)>> const& rhs)
		{
			return lhs.first < rhs.first;
		});
	}

	void render(RenderPassContext& rpc) const override
	{
		for (auto& draw : mDrawList)
		{
			draw.second(rpc);
		}
	}

	void postSettings(RenderPassContext& rpc) override
	{
		mDrawList.clear();
	}

	std::vector<std::pair<u32, std::function<void (RenderPassContext& rpc)>>> mDrawList;
};

class SolidObjectPass : public ObjectPass
{
	LUNA_DECLARE_ABSTRACT(SolidObjectPass, ObjectPass);
public:
	SolidObjectPass(RenderPassContext& rpc) : ObjectPass(rpc) {}
};

class PreComputePass : public ObjectPass
{
	LUNA_DECLARE_ABSTRACT(PreComputePass, ObjectPass);
public:
	PreComputePass(RenderPassContext& rpc) : ObjectPass(rpc) {}
};

class Draw2DPass;
class FluidPass;
class FluidRenderPass;
class ParticleSimulationPass;
class ParticleRenderPass;
class PostEffectPass;
class Renderer : public NonConstructableSingleton<Renderer>
{
public:
	Renderer(ID3D11Device& device, ID3D11DeviceContext& context, std::unordered_map<std::wstring, ShaderResource*>&& map, u32 width, u32 height);

	void setMainCamera(XMFLOAT3 const& position, XMFLOAT3 const& at, XMFLOAT3 const& up);
	void execute();

	PreComputePass& getPreCompute() const;
	SolidObjectPass& getSolid() const; 
	FluidPass& getFluid() const;
	FluidRenderPass& getFluidRender() const;
	Draw2DPass& getDraw2D() const;
	ParticleSimulationPass& getParticleSimulation() const;
	ParticleRenderPass& getParticleRender() const;
	PostEffectPass& getPostEffect() const;

	const RenderPassContext& getContext() const
	{
		return mRPC;
	}
	RenderPassContext& getContext()
	{
		return mRPC;
	}

	void setRenderTarget(SceneLayer layer);

private:
	void updateSystemCBuffer();

private:
	RenderPassContext mRPC;
	vector<unique_ptr<RenderPass>> mPasses;
	array<FrameBuffer, FrameBufferType_Num> mFrameBuffers;
	array<SamplerState, SamplerStateType_Num> mSamplers;
	array<BlendState, BlendStateType_Num> mBlendStates;
	SceneCB mScene;
	ComPtr<ID3D11Buffer> mSceneCB;
};

}
