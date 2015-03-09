#pragma once

#include "app/renderPass.h"
#include "app/renderPass/context.h"
#include "lib/gfx/deviceDX11.h"

namespace luna {

struct FluidResourceSet
{
	std::array<Resource<ID3D11Texture3D>, 3> mVelocities;
	std::array<Resource<ID3D11Texture3D>, 2> mPressures;
	Resource<ID3D11Texture3D> mDivergence;
};

struct FluidRenderResourceSet
{
	ComPtr<ID3D11InputLayout> mIL;
	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;
	u32 mVertexStride;
	u32 mVertexCount;
	u32 mIndexCount;
	ComPtr<ID3D11Buffer> mLineIB;
	u32 mLineIndexCount;
};

struct FluidGrid
{
	XMUINT3 mGridResolution;
	XMFLOAT3 mGridSize;
	XMFLOAT3 mGridPosition;
	XMFLOAT3 mPreviousGridPosition;
	int mJacobiCount;

	FluidResourceSet mResource;
	FluidRenderResourceSet mRenderResource;
	ComPtr<ID3D11Buffer> mCB, mRenderCB;
};

struct PressureSource
{
	XMFLOAT3 mPosition;
	f32 mRadius;
	f32 mPressure;
};

std::shared_ptr<FluidGrid> createFluidGrid(XMUINT3 const& resolution, XMFLOAT3 const& size);

struct FluidGridRegistered
{
	std::shared_ptr<FluidGrid> mGrid;
	bool mReset;
};

class FluidPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(FluidPass, RenderPass);
public:
	FluidPass(RenderPassContext& rpc);

#if 0
	void registerGrid(c8 const* name, std::shared_ptr<FluidGrid> const& grid)
	{
		mGrids.push_back(std::make_pair(name, grid));
	}

	void unregisterGrid(c8 const* name)
	{
		auto it = std::find_if(begin(mGrids), end(mGrids), [name](std::pair<std::string, std::shared_ptr<FluidGrid>> const& p) {
			return p.first == name;
		});
		if (it != end(mGrids)) {
			mGrids.erase(it);
		}
	}
#endif
	void simulateGrid(std::shared_ptr<FluidGrid> const& p, bool reset=false)
	{
		FluidGridRegistered r = { p, reset };
		mGrids.push_back(r);
	}

	void addPressureSource(PressureSource const& source)
	{
		mPressureSources.push_back(source);
	}

private:
	void preSettings(RenderPassContext& rpc) override;
	void render(RenderPassContext& rpc) const override;
	void postSettings(RenderPassContext& rpc) override;

	std::vector<FluidGridRegistered> mGrids;
	std::vector<PressureSource> mPressureSources;
	Resource<ID3D11Buffer> mPressureSource;
	ComPtr<ID3D11SamplerState> mSampler;
};

// for develop
class FluidRenderPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(FluidRenderPass, RenderPass);
public:
	FluidRenderPass(RenderPassContext& rpc);

	void renderGrid(std::shared_ptr<FluidGrid> const& p)
	{
		mGrids.push_back(p);
	}

private:
	void preSettings(RenderPassContext& rpc) override;
	void render(RenderPassContext& rpc) const override;
	void postSettings(RenderPassContext& rpc) override;

	std::vector<std::shared_ptr<FluidGrid>> mGrids;
	ComPtr<ID3D11DepthStencilState> mDSState;
};

}
