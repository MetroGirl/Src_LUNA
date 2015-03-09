#pragma once

#include "lib/gfx/deviceDX11.h"
#include "lib/gfx/frameBuffer.h"
#include "lib/gfx/blendState.h"
#include "app/shaderResource.h"
#include <DirectXMath.h>
#include <unordered_map>

namespace luna {
using namespace DirectX;

struct SceneCB
{
	XMFLOAT4X4 mViewMatrix;
	XMFLOAT4X4 mProjectionMatrix;
	XMFLOAT4X4 mViewProjectionMatrix;
	XMFLOAT4X4 mViewInverseMatrix;
	XMFLOAT4 mEyePosition;
	XMFLOAT2 mResolution;
	XMFLOAT2 mResolutionInv;
	f32 mDemoTimeSecond;
	f32 mDemoTimeRatio;
	f32 mAspectV;
	f32 mAspectH;
	XMFLOAT4 mSceneTimeSecond[SceneLayer_Num];
	XMFLOAT4 mSceneTimeRatio[SceneLayer_Num];
	XMFLOAT4 mSoundFFTBand[36];
	XMFLOAT4 mSoundFFTAvg;
	XMFLOAT4 mSettings;
};

template <class T>
struct Resource
{
	ComPtr<T> mBuffer;
	ComPtr<ID3D11ShaderResourceView> mSRV;
	ComPtr<ID3D11RenderTargetView> mRTV;
	ComPtr<ID3D11UnorderedAccessView> mUAV;
};

struct DepthResource
{
	ComPtr<ID3D11Texture2D> mBuffer;
	ComPtr<ID3D11DepthStencilView> mDSV;
	ComPtr<ID3D11ShaderResourceView> mSRV;
};

struct CanvasResourceSet
{
	Resource<ID3D11Texture2D> mColor;
	DepthResource mDepth;
};

struct ParticleResourceSet
{
	Resource<ID3D11Buffer> mParticle;
	Resource<ID3D11Buffer> mParticle2nd;
	Resource<ID3D11Buffer> mIndexList;
	Resource<ID3D11Buffer> mDeadList;
	Resource<ID3D11Buffer> mIndirectDraw;
};

struct ParticleShadowResourceSet
{
	ComPtr<ID3D11Texture2D> mDepth;
	ComPtr<ID3D11DepthStencilView> mDepthDSV;
	ComPtr<ID3D11ShaderResourceView> mDepthSRV;
};

struct MoonResourceSet
{
	Resource<ID3D11Buffer> mMoonWorldVertexBuffer;
	Resource<ID3D11Buffer> mMoonVertexIndexBuffer;
};

struct FluidGrid;
class ShaderResource;
class RenderPassContext
{
public:
	RenderPassContext(ID3D11Device& device, ID3D11DeviceContext& context, std::unordered_map<std::wstring, ShaderResource*>&& map, const array<FrameBuffer, FrameBufferType_Num>& frameBufferTbl, const array<BlendState, BlendStateType_Num>& blendStateTbl, u32 width, u32 height)
		: mDevice(device)
		, mContext(context)
		, mShaderMap(map)
		, mFrameBufferTbl(frameBufferTbl)
		, mBlendStateTbl(blendStateTbl)
		, mWindowWidth(width), mWindowHeight(height)
		, mFovY(XM_PI/3.f), mNearZ(0.1f), mFarZ(100.f)
		, mDefaultViewport([&](){
				D3D11_VIEWPORT vp;
				vp.TopLeftX = 0;
				vp.TopLeftY = 0;
				vp.Width = static_cast<float>(mWindowWidth);
				vp.Height = static_cast<float>(mWindowHeight);
				vp.MinDepth = 0.f;
				vp.MaxDepth = 1.f;
				return vp;
			}())
		, mMoonVertexCount(0)
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = 16;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		auto hr = device.CreateBuffer(&desc, nullptr, &m16ByteCB);
		if (FAILED(hr)) LUNA_ABORT(L"");

		{
			D3D11_BUFFER_DESC desc = {};
			desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			desc.ByteWidth = 4*4;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA init;
			std::array<u32, 4> zero{{0,0,0,0}};
			init.pSysMem = zero.data();
			init.SysMemPitch = 0;
			init.SysMemSlicePitch = 0;
			auto hr = device.CreateBuffer(&desc, &init, &m4ByteBuffer.mBuffer);
			if (FAILED(hr)) LUNA_ABORT(L"");

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
			uavDesc.Format = DXGI_FORMAT_R32_UINT;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = 4;
			hr = device.CreateUnorderedAccessView(m4ByteBuffer.mBuffer.Get(), &uavDesc, &m4ByteBuffer.mUAV);
			if (FAILED(hr)) LUNA_ABORT(L"");
		}
	}

	ComPtr<ID3D11ShaderResourceView> mMoonAlbedo;
	ComPtr<ID3D11Buffer> m16ByteCB;
	Resource<ID3D11Buffer> m4ByteBuffer;
	std::unordered_map<std::wstring, ShaderResource*> mShaderMap;
	const array<FrameBuffer, FrameBufferType_Num>& mFrameBufferTbl;
	const array<BlendState, BlendStateType_Num>& mBlendStateTbl;
	ParticleResourceSet mParticleResourceSet;
	ParticleShadowResourceSet mParticleShadowResourceSet;
	MoonResourceSet mMoonResourceSet;
	ID3D11Device& mDevice;
	ID3D11DeviceContext& mContext;
	u32 const mWindowWidth, mWindowHeight;
	f32 mFovY, mNearZ, mFarZ;
	std::vector<std::shared_ptr<FluidGrid>> mFluidGrids;
	D3D11_VIEWPORT const mDefaultViewport;
	u32 mMoonVertexCount;

	RenderPassContext(RenderPassContext const&) = delete;
	RenderPassContext& operator = (RenderPassContext const&) = delete;
};

inline
bool applyTechnique(ID3D11DeviceContext& dc, ShaderBlobInfo const& info)
{
	if (info.mCS.shaderPtr) {
		dc.CSSetShader(info.mCS.shaderPtr, nullptr, 0);
	}
	else {
		dc.VSSetShader(info.mVS.shaderPtr, nullptr, 0);
		dc.HSSetShader(info.mHS.shaderPtr, nullptr, 0);
		dc.DSSetShader(info.mDS.shaderPtr, nullptr, 0);
		dc.GSSetShader(info.mGS.shaderPtr, nullptr, 0);
		dc.PSSetShader(info.mPS.shaderPtr, nullptr, 0);
	}
	return true;
}

inline
bool applyTechnique(ID3D11DeviceContext& dc, RenderPassContext const& rpc, c16 const* shaderName, c8 const* technique)
{
	auto it = rpc.mShaderMap.find(shaderName);
	if (it == end(rpc.mShaderMap)) {
		LUNA_WARNINGLINE(L"シェーダーファイル'%ls'が見つかりません", shaderName);
		return false;
	}

	auto shader = it->second->findTechnique(technique);
	if (!shader) {
		LUNA_ERROR(L"シェーダーテクニック'%s'が見つかりません", technique);
		return false;
	}
	return applyTechnique(dc, *shader);
}

}
