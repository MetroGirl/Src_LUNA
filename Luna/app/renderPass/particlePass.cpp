#include "stdafx.h"
#include "app/renderPass/particlePass.h"
#include "app/renderPass/context.h"
#include "app/renderPass/fluidPass.h"
#include "app/shaderResource.h"
#include "app/renderer.h"
#include "lib/gfx/utilityDX11.h"

namespace luna {
namespace {

struct Sort
{
	u32 level;
	u32 levelMask;
	u32 width;
	u32 height;
};

struct KeyIndex
{
	float distance;
	u32 index;
};

struct FluidVelocityCB
{
	XMFLOAT4 mGridPositionMin;
	XMFLOAT4 mGridSize;
};

struct LightMatrixCB
{
	XMFLOAT4X4 mLightViewMatrix;
	XMFLOAT4X4 mLightProjectionMatrix;
};

void createParticleResource(ID3D11Device& device, ParticleResourceSet& r, u32 particleCount)
{
	struct Particle
	{
		XMFLOAT3 position;
		XMFLOAT3 velocity;
		f32 lifeTime;
		XMFLOAT4 color;
//		XMFLOAT3 beginPosition;
//		XMFLOAT3 beginNormal;
		u32 universalIndex;
	};

	struct Particle2nd_Scalable
	{
		XMFLOAT3 beginPosition;
		XMFLOAT3 beginNormal;
	};

	struct Particle2nd_Moon
	{
			u32 targetVertexIndex;
	};

	auto const Particle2ndSize = (std::max)(sizeof(Particle2nd_Scalable), sizeof(Particle2nd_Moon));

	// particle buffer
	{
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(Particle) * particleCount;
		desc.StructureByteStride = sizeof(Particle);
		D3D11_SUBRESOURCE_DATA data;
		std::vector<Particle> initData(particleCount);
		Particle initParticle = {};
		initParticle.lifeTime = -1;
		std::fill(begin(initData), end(initData), initParticle);
		data.pSysMem = initData.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		auto hr = device.CreateBuffer(&desc, &data, &r.mParticle.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"ID3D11Device::CreateBuffer");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = particleCount;
		hr = device.CreateShaderResourceView(r.mParticle.mBuffer.Get(), &srvDesc, &r.mParticle.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"ID3D11Device::CreateShaderResourceView");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = particleCount;
		uavDesc.Buffer.Flags = 0;
		hr = device.CreateUnorderedAccessView(r.mParticle.mBuffer.Get(), &uavDesc, &r.mParticle.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"ID3D11Device::CreateUnorderedAccessView");

		desc.ByteWidth = Particle2ndSize * particleCount;
		desc.StructureByteStride = Particle2ndSize;
		hr = device.CreateBuffer(&desc, nullptr, &r.mParticle2nd.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");

		hr = device.CreateShaderResourceView(r.mParticle2nd.mBuffer.Get(), &srvDesc, &r.mParticle2nd.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");

		hr = device.CreateUnorderedAccessView(r.mParticle2nd.mBuffer.Get(), &uavDesc, &r.mParticle2nd.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}

	// key & index buffer
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(KeyIndex) * particleCount;
		desc.StructureByteStride = sizeof(KeyIndex);
		auto hr = device.CreateBuffer(&desc, nullptr, &r.mIndexList.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"ID3D11Device::CreateBuffer");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = particleCount;
		hr = device.CreateShaderResourceView(r.mIndexList.mBuffer.Get(), &srvDesc, &r.mIndexList.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"ID3D11Device::CreateShaderResourceView");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = particleCount;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_COUNTER;
		hr = device.CreateUnorderedAccessView(r.mIndexList.mBuffer.Get(), &uavDesc, &r.mIndexList.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"ID3D11Device::CreateUnorderedAccessView");
	}

	// dead list
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = /*D3D11_BIND_SHADER_RESOURCE | */D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = 0;//D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(u32) * particleCount;
		desc.StructureByteStride = 0;//sizeof(u32);
		auto hr = device.CreateBuffer(&desc, nullptr, &r.mDeadList.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");
#if 0
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = particleCount;
		hr = device.CreateShaderResourceView(r.mDeadList.mBuffer.Get(), &srvDesc, &r.mDeadList.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");
#endif
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_R32_UINT; //DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = particleCount;
		uavDesc.Buffer.Flags = 0; //D3D11_BUFFER_UAV_FLAG_APPEND;
		hr = device.CreateUnorderedAccessView(r.mDeadList.mBuffer.Get(), &uavDesc, &r.mDeadList.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}

	// indirect draw
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(u32) * (5 + 2);
		desc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA data;
		const struct DrawIndirectArgsAndDeadListCount {
			UINT a, b, c, d, e, deadListCount;
		} args = { 0, 1, 0, 0, 0, particleCount };
		data.pSysMem = &args;
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		auto hr = device.CreateBuffer(&desc, &data, &r.mIndirectDraw.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_UINT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 5 + 2;
		hr = device.CreateShaderResourceView(r.mIndirectDraw.mBuffer.Get(), &srvDesc, &r.mIndirectDraw.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
}

void createParticleShadowResource(ID3D11Device& device, ParticleShadowResourceSet& set)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Quality = 0;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	desc.Width = 4096;
	desc.Height = 4096;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	auto hr = device.CreateTexture2D(&desc, nullptr, &set.mDepth);
	if (FAILED(hr)) LUNA_ERROR(L"");

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.Flags = 0;
	hr = device.CreateDepthStencilView(set.mDepth.Get(), &dsvDesc, &set.mDepthDSV);
	if (FAILED(hr)) LUNA_ERROR(L"");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device.CreateShaderResourceView(set.mDepth.Get(), &srvDesc, &set.mDepthSRV);
	if (FAILED(hr)) LUNA_ERROR(L"");
}

} // unnamed

ParticleSimulationPass::ParticleSimulationPass(RenderPassContext& rpc)
	: mMaxParticleCount(1024 * 1024 * 3) // 3M particles
	, mMaxEmitParticleCount(1024 * 128) // 0.1M
	, mShaderName("particle_simulate")
{
	createParticleResource(rpc.mDevice, rpc.mParticleResourceSet, mMaxParticleCount);

	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;
	desc.CPUAccessFlags = 0;
	desc.ByteWidth = sizeof(ParticleEmitter);
	desc.StructureByteStride = 0;
	auto hr = rpc.mDevice.CreateBuffer(&desc, nullptr, &mEmitterCB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	desc.ByteWidth = sizeof(Sort);
	hr = rpc.mDevice.CreateBuffer(&desc, nullptr, &mSortCB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	desc.ByteWidth = sizeof(FluidVelocityCB);
	hr = rpc.mDevice.CreateBuffer(&desc, nullptr, &mFluidVelocityCB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(KeyIndex) * mMaxParticleCount;
		desc.StructureByteStride = sizeof(KeyIndex);
		auto hr = rpc.mDevice.CreateBuffer(&desc, nullptr, &mSortTmp.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = mMaxParticleCount;
		hr = rpc.mDevice.CreateShaderResourceView(mSortTmp.mBuffer.Get(), &srvDesc, &mSortTmp.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = mMaxParticleCount;
		uavDesc.Buffer.Flags = 0;
		hr = rpc.mDevice.CreateUnorderedAccessView(mSortTmp.mBuffer.Get(), &uavDesc, &mSortTmp.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
	{
		struct xorshift
		{
			xorshift(u32 x, u32 y, u32 z, u32 w)
				: x_(x), y_(y), z_(z), w_(w)
			{}

			u32 random()
			{
				auto t = x_ ^ (x_ << 11);
				x_ = y_; y_ = z_; z_ = w_;
				return w_ = (w_ ^ (w_ >> 19)) ^ (t ^ (t >> 8));
			}
			
		private:
			u32   x_, y_, z_, w_;
		} xorshift(123456789, 36243609, 521288629, 88675123);

		struct uint4 { u32 value_[4]; };
		std::vector<uint4> initial(mMaxEmitParticleCount);
		for(auto& element : initial) {
			element.value_[0] = xorshift.random();
			element.value_[1] = xorshift.random();
			element.value_[2] = xorshift.random();
			element.value_[3] = xorshift.random();
		}
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = sizeof(u32)*4 * mMaxEmitParticleCount;
		desc.StructureByteStride = sizeof(u32)*4;
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = initial.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		auto hr = rpc.mDevice.CreateBuffer(&desc, &data, &mEmitterRandomSeed.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.NumElements = mMaxEmitParticleCount;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = 0;
		hr = rpc.mDevice.CreateUnorderedAccessView(mEmitterRandomSeed.mBuffer.Get(), &uavDesc, &mEmitterRandomSeed.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
}

void ParticleSimulationPass::preSettings(RenderPassContext& rpc)
{}

void ParticleSimulationPass::render(RenderPassContext& rpc) const
{
	auto& dc = rpc.mContext;
	auto& res = rpc.mParticleResourceSet;

	if (mFirstTime) {
		u32 src[4] = { rpc.mMoonVertexCount, 0, 0, 0 };
		dc.UpdateSubresource(rpc.m16ByteCB.Get(), 0, nullptr, src, 0, 0);
		dc.CSSetConstantBuffers(3, 1, rpc.m16ByteCB.GetAddressOf());
		scoped_uav_cs uav(rpc.mContext, 2, {
			rpc.mParticleResourceSet.mDeadList.mUAV.Get(), nullptr,nullptr,
			rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mUAV.Get(), nullptr, rpc.m4ByteBuffer.mUAV.Get() });
		applyTechnique(rpc.mContext, rpc, L"fluid", "particle_init");
		rpc.mContext.Dispatch(mMaxParticleCount / 256, 1, 1);
		if (applyTechnique(dc, rpc, L"fluid", "particle_copy_moon_vertex_dead")) {
			scoped_uav_cs uav(dc, 7, { rpc.m4ByteBuffer.mUAV.Get() });
			dc.Dispatch(1,1,1);
		}
		mFirstTime = false;
	}

	if (!mEmitterControllers.empty())
	{
		u32 src[4] = { rpc.mMoonVertexCount, 0, 0, 0 };
		dc.UpdateSubresource(rpc.m16ByteCB.Get(), 0, nullptr, src, 0, 0);
		dc.CSSetConstantBuffers(3, 1, rpc.m16ByteCB.GetAddressOf());
//		dc.CSSetConstantBuffers(3, 1, rpc.m16ByteCB.GetAddressOf());
		for (auto const& getEmitter : mEmitterControllers) {
			ParticleEmitterSettings emitter;
			getEmitter(emitter);
			if (applyTechnique(dc, rpc, L"fluid", emitter.mShaderName.c_str())) {
				dc.UpdateSubresource(mEmitterCB.Get(), 0, nullptr, &emitter.mEmitter, 0, 0);
				scoped_srv_cs srv(dc, 5, {
					res.mIndirectDraw.mSRV.Get(),
					rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mSRV.Get(),
				});
				scoped_uav_cs uav(dc, 0, {
					res.mParticle.mUAV.Get(),
					nullptr,
					res.mDeadList.mUAV.Get(),
					nullptr,
					mEmitterRandomSeed.mUAV.Get(),
					rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mUAV.Get(),
					rpc.mParticleResourceSet.mParticle2nd.mUAV.Get(),
					rpc.m4ByteBuffer.mUAV.Get(),
				});
			
				dc.CSSetConstantBuffers(1, 1, mEmitterCB.GetAddressOf());
				dc.Dispatch(mMaxEmitParticleCount / 256, 1, 1);
				//dc.CopyStructureCount(res.mIndirectDraw.mBuffer.Get(), 20, res.mDeadList.mUAV.Get());
				//			dc.CopyStructureCount(res.mIndirectDraw.mBuffer.Get(), 24, rpc.m4ByteBuffer.mUAV.Get());//rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mUAV.Get());
			}
			if (applyTechnique(dc, rpc, L"fluid", "particle_copy_moon_vertex_dead")) {
				scoped_uav_cs uav(dc, 7, { rpc.m4ByteBuffer.mUAV.Get() });
				dc.Dispatch(1,1,1);
			}
		}
	}

	{
		if (!rpc.mFluidGrids.empty()) {
			FluidVelocityCB cb;
			XMStoreFloat4(&cb.mGridPositionMin, XMLoadFloat3(&rpc.mFluidGrids[0]->mGridPosition) - XMLoadFloat3(&rpc.mFluidGrids[0]->mGridSize) * 0.5);
			XMStoreFloat4(&cb.mGridSize, XMLoadFloat3(&rpc.mFluidGrids[0]->mGridSize));
			dc.UpdateSubresource(mFluidVelocityCB.Get(), 0, nullptr, &cb, 0, 0);
			dc.CSSetConstantBuffers(2, 1, mFluidVelocityCB.GetAddressOf());
		}

		UINT const keepOffset = -1;
		scoped_srv_cs srv(dc, 3, {
			rpc.mFluidGrids.empty()? nullptr: rpc.mFluidGrids[0]->mResource.mVelocities[0].mSRV.Get(),
			nullptr, nullptr,
			rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mSRV.Get(),
		});
		scoped_uav_cs uav(dc, 0, {
			res.mParticle.mUAV.Get(),
			res.mIndexList.mUAV.Get(),
			res.mDeadList.mUAV.Get(),
			nullptr,
			nullptr,
			rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mUAV.Get(),
			rpc.mParticleResourceSet.mParticle2nd.mUAV.Get(),
			rpc.m4ByteBuffer.mUAV.Get(),
		}, { keepOffset, 0, keepOffset, keepOffset, keepOffset, keepOffset, keepOffset, keepOffset });
		applyTechnique(dc, rpc, L"fluid", mShaderName.c_str());
		dc.Dispatch(mMaxParticleCount/256, 1, 1);
	}
	dc.CopyStructureCount(res.mIndirectDraw.mBuffer.Get(), 0, res.mIndexList.mUAV.Get());

#if 1 // work on, particle sort
	if(SettingsManager::instance().isLowQuality()){
		dc.CSSetConstantBuffers(1, 1, mSortCB.GetAddressOf());

		auto const matrixWidth = 512;
		auto const matrixHeight = mMaxParticleCount / 512;
		for (u32 level=2; level <= 512; level <<= 1) {
			Sort cb = { level, level, matrixWidth, matrixHeight };
			dc.UpdateSubresource(mSortCB.Get(), 0, nullptr, &cb, 0, 0);
			scoped_uav_cs uav(dc, 0, { res.mIndexList.mUAV.Get() });
			applyTechnique(dc, rpc, L"fluid", "particle_sort");
			dc.Dispatch(mMaxParticleCount/512, 1, 1);
		}
		for (u32 level = (512 << 1); level <= mMaxParticleCount; level <<= 1) {
			{
				Sort cb1 = { level / 512, (level&~mMaxParticleCount) / 512, matrixWidth, matrixHeight };
				dc.UpdateSubresource(mSortCB.Get(), 0, nullptr, &cb1, 0, 0);
				scoped_srv_cs srv(dc, 0, {res.mIndexList.mSRV.Get()});
				scoped_uav_cs uav(dc, 0, {mSortTmp.mUAV.Get()});
				applyTechnique(dc, rpc, L"fluid", "particle_sort_transpose");
				dc.Dispatch(matrixWidth/16, matrixHeight/16, 1);
				applyTechnique(dc, rpc, L"fluid", "particle_sort");
				dc.Dispatch(mMaxParticleCount/512, 1, 1);
			}
			{
				Sort cb2 = { 512, level, matrixHeight, matrixWidth };
				dc.UpdateSubresource(mSortCB.Get(), 0, nullptr, &cb2, 0, 0);
				scoped_srv_cs srv(dc, 0, {mSortTmp.mSRV.Get()});
				scoped_uav_cs uav(dc, 0, {res.mIndexList.mUAV.Get()});
				applyTechnique(dc, rpc, L"fluid", "particle_sort_transpose");
				dc.Dispatch(matrixHeight/16, matrixWidth/16, 1);
				applyTechnique(dc, rpc, L"fluid", "particle_sort");
				dc.Dispatch(mMaxParticleCount/512, 1, 1);
			}
		}
	}
#endif
}

void ParticleSimulationPass::postSettings(RenderPassContext& rpc)
{
	mEmitterControllers.clear();
}

ParticleShadowPass::ParticleShadowPass(RenderPassContext& rpc)
{
	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(LightMatrixCB);
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	auto hr = rpc.mDevice.CreateBuffer(&desc, nullptr, &mCB);
	if (FAILED(hr)) LUNA_ERROR(L"");
	{
		CD3D11_RASTERIZER_DESC desc(D3D11_DEFAULT);

	}
	createParticleShadowResource(rpc.mDevice, rpc.mParticleShadowResourceSet);
}

ParticleShadowPass::~ParticleShadowPass()
{}

void ParticleShadowPass::preSettings(RenderPassContext& rpc)
{}

void ParticleShadowPass::render(RenderPassContext& rpc) const
{
#if 0
	// ƒ‰ƒCƒg‚Í‚Æ‚è‚ ‚¦‚¸ŒÅ’è‚Å
	XMFLOAT3 pos(0, 10, 0), at(0, 0, 0), up(1, 0, 0);
	auto lookAt = XMMatrixLookAtLH(XMLoadFloat3(&pos), XMLoadFloat3(&at), XMLoadFloat3(&up));
	auto projection = XMMatrixPerspectiveFovLH(XM_PI/3.f, 1.f, 0.1f, 100.f);
	LightMatrixCB cb;
	XMStoreFloat4x4(&cb.mLightProjectionMatrix, XMMatrixTranspose(lookAt));
	XMStoreFloat4x4(&cb.mLightProjectionMatrix, XMMatrixTranspose(projection));
	auto& dc = rpc.mContext;
	dc.UpdateSubresource(mCB.Get(), 0, nullptr, &cb, 0, 0);

	auto& resource = rpc.mParticleShadowResourceSet;

	if (applyTechnique(dc, rpc, L"fluid", "particle_shadow")) {
		dc.GSSetConstantBuffers(1, 1, mCB.GetAddressOf());
		dc.ClearDepthStencilView(resource.mDepthDSV.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = 4096;
		viewport.Height = 4096;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		dc.RSSetViewports(1, &viewport);
		dc.OMSetRenderTargets(0, nullptr, resource.mDepthDSV.Get());
		dc.IASetInputLayout(nullptr);
		dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		dc.DrawInstancedIndirect(rpc.mParticleResourceSet.mIndirectDraw.mBuffer.Get(), 0);

		Renderer::instance().setRenderTarget(SceneLayer_0);
		dc.RSSetViewports(1, &rpc.mDefaultViewport);
	}
#endif
}

void ParticleShadowPass::postSettings(RenderPassContext& rpc)
{}

ParticleRenderPass::ParticleRenderPass(RenderPassContext& rpc)
	: mParticleTexture(ResourceManager::instance().load<TextureResource>(L"data/particle.dds"))
	, mShaderName("particle_render")
{
	{
		D3D11_BLEND_DESC desc;
		desc.AlphaToCoverageEnable = FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = TRUE;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		auto hr = rpc.mDevice.CreateBlendState(&desc, &mBlendState);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		auto hr = rpc.mDevice.CreateDepthStencilState(&desc, &mDepthStencilState);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
	{
		CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
		desc.AddressU = desc.AddressV = desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		auto hr = rpc.mDevice.CreateSamplerState(&desc, &mShadowSampler);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
}

ParticleRenderPass::~ParticleRenderPass()
{
	if (mParticleTexture) {
		mParticleTexture->release();
	}
}

void ParticleRenderPass::preSettings(RenderPassContext& rpc)
{
}

void ParticleRenderPass::postSettings(RenderPassContext& rpc)
{
}

void ParticleRenderPass::render(RenderPassContext& rpc) const
{
	auto& dc = rpc.mContext;
	auto& res = rpc.mParticleResourceSet;

	applyTechnique(dc, rpc, L"fluid", mShaderName.c_str());
	dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	dc.IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	dc.IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	dc.IASetInputLayout(nullptr);
	scoped_srv_vs vssrv(dc, 0, {
		res.mParticle.mSRV.Get(),
		res.mIndexList.mSRV.Get(),
		nullptr,
		rpc.mParticleShadowResourceSet.mDepthSRV.Get(),
		nullptr,
		nullptr,
		rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mSRV.Get(),
		rpc.mMoonAlbedo.Get(),
	});
	scoped_srv_ps pssrv(dc, 2, { &mParticleTexture->getSRV() });
	f32 blendFactor[] = { 0.f, 0.f, 0.f, 0.f };
	dc.OMSetBlendState(mBlendState.Get(), blendFactor, 0xffffffff);
	dc.OMSetDepthStencilState(mDepthStencilState.Get(), 0);
	dc.DrawInstancedIndirect(res.mIndirectDraw.mBuffer.Get(), 0);
}

}
