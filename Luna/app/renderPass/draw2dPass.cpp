#include "stdafx.h"
#include "app/renderPass/draw2dPass.h"
#include "app/renderPass/context.h"
#include "app/shaderResource.h"

namespace luna {
LUNA_IMPLEMENT_ABSTRACT(luna::Draw2DPass);

Draw2DPass::Draw2DPass(RenderPassContext& rpc)
{
	mBatch.reserve(1024);
	mVertexBuffer.reserve(1024);
	mIndexBuffer.reserve(1024);

	auto& device = rpc.mDevice;
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = 1024 * 1024 * 4; // 4 MiB
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		auto hr = device.CreateBuffer(&desc, nullptr, &mVB);
		if (FAILED(hr)) LUNA_ERROR(L"Darw2DPass::Draw2DPass: create vertex buffer failed.");
	}
	{
		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.ByteWidth = 1024 * 1024 * 4; // 4 MiB
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		auto hr = device.CreateBuffer(&desc, nullptr, &mIB);
		if (FAILED(hr)) LUNA_ERROR(L"Darw2DPass::Draw2DPass: create index buffer failed.");
	}
	{
		D3D11_INPUT_ELEMENT_DESC const vertexLayout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		auto it = rpc.mShaderMap.find(L"develop");
		if (it == rpc.mShaderMap.end()) LUNA_ERROR(L"Draw2DPass::Draw2DPass: shader 'develop' not found.");

		auto* info = it->second->findTechnique("develop2d_color");
		auto hr = device.CreateInputLayout(vertexLayout, ARRAYSIZE(vertexLayout), info->mVS.blobPtr->GetBufferPointer(), info->mVS.blobPtr->GetBufferSize(), &mIL);
		if (FAILED(hr)) LUNA_ERROR(L"Draw2DPass::Draw2DPass: create input layout failed.");
	}
}

void Draw2DPass::drawText()
{}

void Draw2DPass::drawRectangle(s32 ux, s32 uy, s32 uwidth, s32 uheight, u32 color)
{
	Batch batch;
	batch.mType = BatchType::Triangle;
	batch.mVertexCount = 4;
	batch.mIndexCount = 6;
	mBatch.push_back(batch);
	
	auto x = static_cast<f32>(ux);
	auto y = static_cast<f32>(uy);
	auto width = static_cast<f32>(uwidth);
	auto height = static_cast<f32>(uheight);
	Vertex v[] = {
		{ x, y, 0.f, 1.f, color },
		{ x+width, y, 0.f, 1.f, color },
		{ x+width, y+height, 0.f, 1.f, color },
		{ x, y+height, 0.f, 1.f, color },
	};
	mVertexBuffer.insert(end(mVertexBuffer), std::begin(v), std::end(v));

	u16 i[] = {
		0, 1, 2, 0, 2, 3
	};
	mIndexBuffer.insert(end(mIndexBuffer), std::begin(i), std::end(i));
}

void Draw2DPass::preSettings(RenderPassContext& rpc)
{
	// pre transform vertices
	for (auto& v : mVertexBuffer) {
		v.x = ((v.x + 0.5f) / (rpc.mWindowWidth * 0.5f)) - 1.f;
		v.y = 1.f - ((v.y + 0.5f) / (rpc.mWindowHeight * 0.5f));
	}
}

void Draw2DPass::render(RenderPassContext& rpc) const
{
	auto& dc = rpc.mContext;

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(dc.Map(mVB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))){
		memcpy(mapped.pData, mVertexBuffer.data(), sizeof(Vertex)*mVertexBuffer.size());
		dc.Unmap(mVB.Get(), 0);
	}
	if (SUCCEEDED(dc.Map(mIB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))){
		memcpy(mapped.pData, mIndexBuffer.data(), sizeof(u16)*mIndexBuffer.size());
		dc.Unmap(mIB.Get(), 0);
	}

	auto stride = sizeof(Vertex);
	auto offset = 0U;
	dc.IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
	dc.IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R16_UINT, 0);
	dc.IASetInputLayout(mIL.Get());

	auto it = rpc.mShaderMap.find(L"develop");
	if (it == rpc.mShaderMap.end()) LUNA_ERROR(L"Draw2DPass::Draw2DPass: shader 'develop' not found.");

	auto* info = it->second->findTechnique("develop2d_color");
	dc.VSSetShader(info->mVS.shaderPtr, nullptr, 0);
	dc.HSSetShader(nullptr, nullptr, 0);
	dc.DSSetShader(nullptr, nullptr, 0);
	dc.GSSetShader(nullptr, nullptr, 0);
	dc.PSSetShader(info->mPS.shaderPtr, nullptr, 0);

	u32 startIndexLocation = 0;
	s32 baseVertexLocation = 0;
	for (auto const& batch : mBatch)
	{
		if (batch.mType == BatchType::Triangle) {
			dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc.DrawIndexed(batch.mIndexCount, startIndexLocation, baseVertexLocation);
		}
		else if (batch.mType == BatchType::Line) {

		}
		startIndexLocation += batch.mIndexCount;
		baseVertexLocation += batch.mVertexCount;
	}
}

void Draw2DPass::postSettings(RenderPassContext&)
{
	mBatch.clear();
	mVertexBuffer.clear();
	mIndexBuffer.clear();
}

}
