#include "stdafx.h"
#include "app/renderPass/fluidPass.h"
#include "app/renderer.h"
#include "lib/gfx/utilityDX11.h"

namespace luna {
namespace {

struct CB
{
	XMUINT4 mGridResolution;
	XMFLOAT4 mGridResolutionInv;
	XMFLOAT4 mGridSize;
	XMFLOAT4 mGridPositionMin;
	XMFLOAT4 mMovingVelocity;
	float mTimeStep;
	int mUseVorticityConfinement;
	float mVorticityCoeff;
	u32 mPressureSourceCount;
};

struct RenderCB
{
	XMFLOAT4X4 mWorldMatrix;
};

void createFluidResources(ID3D11Device& device, FluidResourceSet& r, XMUINT3 const& gridResolution)
{
	{
		std::vector<u16> initVolume1(gridResolution.x*gridResolution.y*gridResolution.z);
		std::vector<u16> initVolume4(gridResolution.x*gridResolution.y*gridResolution.z*4);
		std::fill(begin(initVolume1), end(initVolume1), 0);
		std::fill(begin(initVolume4), end(initVolume4), 0);

		D3D11_SUBRESOURCE_DATA initData1, initData4;
		initData1.pSysMem = initVolume1.data();
		initData1.SysMemPitch = gridResolution.x*sizeof(u16);
		initData1.SysMemSlicePitch = gridResolution.x*gridResolution.y*sizeof(u16);
		initData4.pSysMem = initVolume4.data();
		initData4.SysMemPitch = gridResolution.x*sizeof(u16)*4;
		initData4.SysMemSlicePitch = gridResolution.x*gridResolution.y*sizeof(u16)*4;

		D3D11_TEXTURE3D_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Width = gridResolution.x;
		desc.Height = gridResolution.y;
		desc.Depth = gridResolution.z;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;

		for (size_t i = 0; i < r.mVelocities.size(); ++i) {
			auto hr = device.CreateTexture3D(&desc, &initData4, &r.mVelocities[i].mBuffer);
			if (FAILED(hr)) LUNA_ERROR(L"");
			hr = device.CreateShaderResourceView(r.mVelocities[i].mBuffer.Get(), nullptr, &r.mVelocities[i].mSRV);
			if (FAILED(hr)) LUNA_ERROR(L"");
			hr = device.CreateUnorderedAccessView(r.mVelocities[i].mBuffer.Get(), nullptr, &r.mVelocities[i].mUAV);
			if (FAILED(hr)) LUNA_ERROR(L"");
		}

		desc.Format = DXGI_FORMAT_R16_FLOAT;
		for (size_t i = 0; i < r.mPressures.size(); ++i) {
			auto hr = device.CreateTexture3D(&desc, &initData1, &r.mPressures[i].mBuffer);
			if(FAILED(hr)) LUNA_ERROR(L"");
			hr = device.CreateShaderResourceView(r.mPressures[i].mBuffer.Get(), nullptr, &r.mPressures[i].mSRV);
			if(FAILED(hr)) LUNA_ERROR(L"");
			hr = device.CreateUnorderedAccessView(r.mPressures[i].mBuffer.Get(), nullptr, &r.mPressures[i].mUAV);
			if(FAILED(hr)) LUNA_ERROR(L"");
		}

		auto hr = device.CreateTexture3D(&desc, &initData1, &r.mDivergence.mBuffer);
		if(FAILED(hr)) LUNA_ERROR(L"");
		hr = device.CreateShaderResourceView(r.mDivergence.mBuffer.Get(), nullptr, &r.mDivergence.mSRV);
		if(FAILED(hr)) LUNA_ERROR(L"");
		hr = device.CreateUnorderedAccessView(r.mDivergence.mBuffer.Get(), nullptr, &r.mDivergence.mUAV);
		if(FAILED(hr)) LUNA_ERROR(L"");
	}
}

void createFluidRenderResource(ID3D11Device& device, FluidRenderResourceSet& r, RenderPassContext& rpc)
{
	struct Vertex { f32 x, y, z; };
	Vertex const VolumeVertices[] = {
		{ -1.f,  1.f,  1.f, },
		{  1.f,  1.f,  1.f, },
		{  1.f, -1.f,  1.f, },
		{ -1.f, -1.f,  1.f, },
		{  1.f,  1.f, -1.f, },
		{ -1.f,  1.f, -1.f, },
		{ -1.f, -1.f, -1.f, },
		{  1.f, -1.f, -1.f, },
	};

	unsigned short const VolumeIndices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		5, 4, 1, 5, 1, 0,
		7, 6, 3, 7, 3, 2,
		1, 4, 7, 1, 7, 2,
		5, 0, 3, 5, 3, 6,
	};

	unsigned short const GridLineIndices[] = {
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 5, 1, 4, 2, 7, 3, 6,
	};

	D3D11_INPUT_ELEMENT_DESC const VolumeInputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	r.mVertexStride = sizeof(Vertex);
	r.mVertexCount = ARRAYSIZE(VolumeVertices);
	r.mIndexCount = ARRAYSIZE(VolumeIndices);
	r.mLineIndexCount = ARRAYSIZE(GridLineIndices);

	auto* info = rpc.mShaderMap.find(L"fluid")->second->findTechnique("fluid_render_volume");
	auto hr = device.CreateInputLayout(VolumeInputDesc, ARRAYSIZE(VolumeInputDesc),
		info->mVS.blobPtr->GetBufferPointer(), info->mVS.blobPtr->GetBufferSize(), &r.mIL);
	if (FAILED(hr)) LUNA_ERROR(L"");

	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(Vertex) * ARRAYSIZE(VolumeVertices);
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA init;
	init.pSysMem = VolumeVertices;
	init.SysMemPitch = 0;
	init.SysMemSlicePitch = 0;
	hr = device.CreateBuffer(&desc, &init, &r.mVB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(u16) * ARRAYSIZE(VolumeIndices);
	init.pSysMem = VolumeIndices;
	hr = device.CreateBuffer(&desc, &init, &r.mIB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	desc.ByteWidth = sizeof(u16) * ARRAYSIZE(GridLineIndices);
	init.pSysMem = GridLineIndices;
	hr = device.CreateBuffer(&desc, &init, &r.mLineIB);
	if (FAILED(hr)) LUNA_ERROR(L"");
}

void execute(RenderPassContext& rpc, FluidGrid& grid, Resource<ID3D11Buffer> const& pressureSource, u32 pressureSourceCount, bool reset)
{
	XMUINT3 const n(grid.mGridResolution.x/16, grid.mGridResolution.y/4, grid.mGridResolution.z/4);
	auto& dc = rpc.mContext;
	auto& r = grid.mResource;

	{
		CB cb;
		cb.mGridResolution = XMUINT4(grid.mGridResolution.x, grid.mGridResolution.y, grid.mGridResolution.z, 0);
		cb.mGridResolutionInv = XMFLOAT4(1.f/grid.mGridResolution.x, 1.f/grid.mGridResolution.y, 1.f/grid.mGridResolution.z, 0.f);
		cb.mGridSize = XMFLOAT4(grid.mGridSize.x, grid.mGridSize.y, grid.mGridSize.z, 0.f);
		XMStoreFloat4(&cb.mGridPositionMin, XMLoadFloat3(&grid.mGridPosition) - XMLoadFloat3(&grid.mGridSize)*0.5f);
		cb.mTimeStep = 0.f;
		cb.mUseVorticityConfinement = 0;
		cb.mPressureSourceCount = pressureSourceCount;
		XMStoreFloat4(&cb.mMovingVelocity, XMLoadFloat3(&grid.mGridPosition) - XMLoadFloat3(&grid.mPreviousGridPosition));
		dc.UpdateSubresource(grid.mCB.Get(), 0, nullptr, &cb, 0, 0);
	}

	dc.CSSetConstantBuffers(1, 1, grid.mCB.GetAddressOf());

	if (reset) {
		if (applyTechnique(dc, rpc, L"fluid", "fluid_reset")) {
			scoped_uav_cs uav(dc, 0, { r.mVelocities[0].mUAV.Get(),nullptr,nullptr,nullptr, r.mPressures[0].mUAV.Get() });
			dc.Dispatch(n.x, n.y, n.z);
		}
	}

	if (applyTechnique(dc, rpc, L"fluid", "fluid_advect")) {
		scoped_srv_cs srv(dc, 0, { r.mVelocities[0].mSRV.Get() });
		scoped_uav_cs uav(dc, 1, { r.mVelocities[1].mUAV.Get() });
		dc.Dispatch(n.x, n.y, n.z);
	}
	if (applyTechnique(dc, rpc, L"fluid", "fluid_advect_backward")) {
		scoped_srv_cs srv(dc, 0, { r.mVelocities[0].mSRV.Get(), r.mVelocities[1].mSRV.Get() });
		scoped_uav_cs uav(dc, 2, { r.mVelocities[2].mUAV.Get() });
		dc.Dispatch(n.x, n.y, n.z);
	}
	if (applyTechnique(dc, rpc, L"fluid", "fluid_advect_maccormack")) {
		scoped_srv_cs srv(dc, 0, { r.mVelocities[0].mSRV.Get(), nullptr, r.mVelocities[2].mSRV.Get() });
		scoped_uav_cs uav(dc, 1, { r.mVelocities[1].mUAV.Get() });
		dc.Dispatch(n.x, n.y, n.z);
	}
	if (applyTechnique(dc, rpc, L"fluid", "fluid_divergence")) {
		scoped_srv_cs srv(dc, 0, { r.mVelocities[0].mSRV.Get() });
		scoped_uav_cs uav(dc, 3, { r.mDivergence.mUAV.Get() });
		dc.Dispatch(n.x, n.y, n.z);
	}
	if (applyTechnique(dc, rpc, L"fluid", "fluid_jacobi")) {
		for (int i = 0; i < 32/*grid.mJacobiCount*/; ++i) {
			scoped_srv_cs srv(dc, 3, { r.mDivergence.mSRV.Get(), r.mPressures[0].mSRV.Get() });
			scoped_uav_cs uav(dc, 4, { r.mPressures[1].mUAV.Get() });
			dc.Dispatch(n.x, n.y, n.z);
			std::swap(r.mPressures[0], r.mPressures[1]);
		}
	}
	if (0 < pressureSourceCount) {
		if(applyTechnique(dc, rpc, L"fluid", "fluid_add_pressure")) {
			scoped_srv_cs srv(dc, 6, { pressureSource.mSRV.Get() });
			scoped_uav_cs uav(dc, 4, { r.mPressures[0].mUAV.Get() });
			dc.Dispatch(n.x, n.y, n.z);
		}
	}
	if (applyTechnique(dc, rpc, L"fluid", "fluid_project")) {
		scoped_srv_cs srv(dc, 1, { r.mVelocities[1].mSRV.Get(), nullptr, nullptr, r.mPressures[0].mSRV.Get() });
		scoped_uav_cs uav(dc, 0, { r.mVelocities[0].mUAV.Get() });
		dc.Dispatch(n.x, n.y, n.z);
	}
}

void resetFluid(ID3D11DeviceContext& dc, FluidGrid& grid)
{
	
}

void renderVolume(RenderPassContext& rpc, FluidGrid const& grid)
{
	u32 offset = 0;
	auto& r = grid.mRenderResource;
	auto& dc = rpc.mContext;

	RenderCB cb;
	auto translate = XMMatrixTranslation(grid.mGridPosition.x, grid.mGridPosition.y, grid.mGridPosition.z);
	auto scale = XMMatrixScaling(grid.mGridSize.x, grid.mGridSize.y, grid.mGridSize.z);
	XMStoreFloat4x4(&cb.mWorldMatrix, XMMatrixTranspose(scale * translate));
	dc.UpdateSubresource(grid.mRenderCB.Get(), 0, nullptr, &cb, 0, 0);
	dc.VSSetConstantBuffers(2, 1, grid.mRenderCB.GetAddressOf());
	dc.PSSetConstantBuffers(1, 1, grid.mCB.GetAddressOf());

	if (applyTechnique(dc, rpc, L"fluid", "fluid_render_volume")) {
		scoped_srv_ps srv(dc, 0, { grid.mResource.mVelocities[0].mSRV.Get() });
		dc.IASetInputLayout(r.mIL.Get());
		dc.IASetVertexBuffers(0, 1, r.mVB.GetAddressOf(), &r.mVertexStride, &offset);
		dc.IASetIndexBuffer(r.mIB.Get(), DXGI_FORMAT_R16_UINT, 0);
		dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//		dc.DrawIndexed(r.mIndexCount, 0, 0);
	}
	if (applyTechnique(dc, rpc, L"fluid", "fluid_render_grid")) {
		dc.IASetInputLayout(r.mIL.Get());
		dc.IASetVertexBuffers(0, 1, r.mVB.GetAddressOf(), &r.mVertexStride, &offset);
		dc.IASetIndexBuffer(r.mLineIB.Get(), DXGI_FORMAT_R16_UINT, 0);
		dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
#if !LUNA_PUBLISH
		dc.DrawIndexed(r.mLineIndexCount, 0, 0);
#endif
	}
}

} // unnamed

std::shared_ptr<FluidGrid> createFluidGrid(XMUINT3 const& resolution, XMFLOAT3 const& size)
{
	auto& device = GraphicsDevice::instance().getDevice();
	auto p = std::make_shared<FluidGrid>();
	p->mGridResolution = resolution;
	p->mGridSize = size;
	p->mJacobiCount = 8;

	createFluidResources(device, p->mResource, resolution);
	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(CB);
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	auto hr = device.CreateBuffer(&desc, nullptr, &p->mCB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	createFluidRenderResource(device, p->mRenderResource, /*TODO: fix*/ const_cast<RenderPassContext&>(Renderer::instance().getContext()));
	desc.ByteWidth = sizeof(RenderCB);
	hr = device.CreateBuffer(&desc, nullptr, &p->mRenderCB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	return p;
}

FluidPass::FluidPass(RenderPassContext& rpc)
{
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = sizeof(PressureSource) * 16;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.StructureByteStride = sizeof(PressureSource);
		auto hr = rpc.mDevice.CreateBuffer(&desc, nullptr, &mPressureSource.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = 16;
		hr = rpc.mDevice.CreateShaderResourceView(mPressureSource.mBuffer.Get(), &srvDesc, &mPressureSource.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
	{
	CD3D11_SAMPLER_DESC desc(D3D11_DEFAULT);
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = desc.BorderColor[1] = desc.BorderColor[2] = desc.BorderColor[3] = 0.f;
	auto hr = rpc.mDevice.CreateSamplerState(&desc, &mSampler);
	if (FAILED(hr)) LUNA_ERROR(L"");
	}
}

void FluidPass::preSettings(RenderPassContext& rpc)
{}

void FluidPass::render(RenderPassContext& rpc) const
{
	rpc.mContext.CSSetSamplers(0, 1, mSampler.GetAddressOf());
	if (!mPressureSources.empty()) {
		rpc.mContext.UpdateSubresource(mPressureSource.mBuffer.Get(), 0, nullptr, mPressureSources.data(), 0, 0);
	}
	for (auto const& registered : mGrids) {
		if (registered.mGrid) {
			execute(rpc, *registered.mGrid, mPressureSource, static_cast<u32>(mPressureSources.size()), registered.mReset);
		}
	}
}

void FluidPass::postSettings(RenderPassContext& rpc)
{
	for (auto& registered : mGrids) {
		rpc.mFluidGrids.push_back(registered.mGrid);
	}
	mGrids.clear();
	mPressureSources.clear();
}



FluidRenderPass::FluidRenderPass(RenderPassContext& rpc)
{
	{
	CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
	desc.DepthEnable = FALSE;
	auto hr = rpc.mDevice.CreateDepthStencilState(&desc, &mDSState);
	if (FAILED(hr)) LUNA_ERROR(L"");
	}
}

void FluidRenderPass::preSettings(RenderPassContext& rpc)
{}

void FluidRenderPass::render(RenderPassContext& rpc) const
{
	for (auto const& grid : mGrids) {
		if (grid) {
			//rpc.mContext.OMSetDepthStencilState(mDSState.Get(), 0);
			renderVolume(rpc, *grid);
		}
	}
}

void FluidRenderPass::postSettings(RenderPassContext& rpc)
{
	mGrids.clear();
}

}
