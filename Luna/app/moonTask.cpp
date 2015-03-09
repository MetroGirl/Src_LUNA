#include "stdafx.h"
#include "moonTask.h"
#include "renderer.h"
#include "renderPass/particlePass.h"
#include "renderPass/draw2dPass.h"
#include "stringHelper.h"
#include "lib/gfx/utilityDX11.h"
#include <vector>

namespace luna {
namespace {

template <class VertexT>
void make_sphere_primitive(unsigned int slice, unsigned int stack,
                           std::vector<VertexT>&        vertices,
                           std::vector<u32>&  indices)
{
  assert(2 <= stack);

  auto const vertex_count = slice * (stack-1) + 2;
  auto const index_count  = ((slice * (stack-2) * 2) + (slice * 2)) * 3;

  vertices.reserve(vertex_count);
  indices.reserve(index_count);

  VertexT v;

  auto const du = DirectX::XM_2PI / slice;
  auto const dv = DirectX::XM_PI / stack;
	auto const ds = 1.f / slice;
	auto const dt = 1.f / stack;

  // north
  v.position_[0] = 0.f;
  v.position_[1] = 1.f;
  v.position_[2] = 0.f;
  v.normal_ = v.position_;
	v.uv_[0] = 0.f;
	v.uv_[1] = 1.f;
  vertices.push_back(v);
  // body
  for (unsigned int j = 0; j < (stack-1); ++j) {
    for (unsigned int i = 0; i < slice; ++i) {
      float theta = du * i;
      float phi   = dv * (j+1);
      v.position_[0] = -std::sin(theta) * std::sin(phi);
      v.position_[1] = std::cos(phi);
      v.position_[2] = std::cos(theta) * std::sin(phi);
      v.normal_ = v.position_;
			v.uv_[0] = ds * i;
			v.uv_[1] = 1.f - dt * (j+1);
      vertices.push_back(v);
    }
  }
  // south
  v.position_[0] = 0.f;
  v.position_[1] = -1.f;
  v.position_[2] = 0.f;
  v.normal_ = v.position_;
	v.uv_[0] = 1.f;
	v.uv_[1] = 0.f;
  vertices.push_back(v);

  // north
  auto begin = 1;
  for (unsigned int i = 0; i < slice; ++i) {
    indices.push_back(0);
    indices.push_back(((i+1)%slice)+1);
    indices.push_back(i+1);
  }
  // body
  for (unsigned int i = 0; i < (stack-2); ++i) {
    for (unsigned int j = 0; j < slice; ++j) {
      indices.push_back(begin + j);
      indices.push_back(begin + ((j+1)%slice));
      indices.push_back(begin + slice + ((j+1)%slice));
      indices.push_back(begin + j);
      indices.push_back(begin + slice + ((j+1)%slice));
      indices.push_back(begin + slice + j);
    }
    begin += slice;
  }
  // south
  //begin = slice * (stack-1) + 1;
  for (unsigned int i = 0; i < slice; ++i) {
    indices.push_back(begin+i);
    indices.push_back(begin+((i+1)%slice));
    indices.push_back(vertices.size()-1);
  }
}

ID3D11InputLayout* create_normal_input_layout(ID3D11Device& device, ID3DBlob& blob)
{
  D3D11_INPUT_ELEMENT_DESC desc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  ID3D11InputLayout* layout;
  auto hr = device.CreateInputLayout(desc, ARRAYSIZE(desc), blob.GetBufferPointer(), blob.GetBufferSize(), &layout);
	if (FAILED(hr)) LUNA_ERROR(L"");
  return layout;
}

struct Vertex {
	std::array<f32, 3> position_;
	std::array<f32, 3> normal_;
	std::array<f32, 2> uv_;
};

}

LUNA_IMPLEMENT_CONCRETE(luna::MoonTask);

MoonTask::MoonTask()
	: mDispatchFlag(false)
{
	auto& device = GraphicsDevice::instance().getDevice();
	auto it = Renderer::instance().getContext().mShaderMap.find(L"environment");
	auto const* tech = it->second->findTechnique("moon");
	mIL = create_normal_input_layout(device, *tech->mVS.blobPtr);

	std::vector<Vertex> vertices;
	std::vector<u32> indices;
	u32 const slice = 256, stack = 256;
	make_sphere_primitive<Vertex>(slice, stack, vertices, indices);

	CD3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(Vertex) * vertices.size();
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = vertices.data();
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;
	auto hr = device.CreateBuffer(&desc, &data, &mVB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.NumElements = vertices.size()*sizeof(Vertex)/4;
	srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
	hr = device.CreateShaderResourceView(mVB.Get(), &srvDesc, &mVBSRV);
	if (FAILED(hr)) LUNA_ERROR(L"");

	desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	desc.ByteWidth = sizeof(u32) * indices.size();
	desc.MiscFlags = 0;
	data.pSysMem = indices.data();
	hr = device.CreateBuffer(&desc, &data, &mIB);
	if (FAILED(hr)) LUNA_ERROR(L"");

	mVertexCount = vertices.size();
	mIndexCount = indices.size();

	auto& rpc = Renderer::instance().getContext();
	rpc.mMoonVertexCount = mVertexCount;
	{
		CD3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Vertex) * vertices.size();
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(Vertex);
		hr = device.CreateBuffer(&desc, nullptr, &rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = mVertexCount;
		hr = device.CreateShaderResourceView(rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mBuffer.Get(), &srvDesc, &rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = mVertexCount;
		uavDesc.Buffer.Flags = 0;
		hr = device.CreateUnorderedAccessView(rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mBuffer.Get(), &uavDesc, &rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
	{
		u32 const particleCount = 1024*1024*3;
		CD3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(u32) * particleCount;//vertices.size();
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;//D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = 0;//sizeof(u32);
		hr = device.CreateBuffer(&desc, nullptr, &rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mBuffer);
		if (FAILED(hr)) LUNA_ERROR(L"");
#if 0
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32_UINT;//DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = mVertexCount;
		hr = device.CreateShaderResourceView(rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mBuffer.Get(), &srvDesc, &rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mSRV);
		if (FAILED(hr)) LUNA_ERROR(L"");
#endif
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_R32_UINT;//DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = particleCount;//mVertexCount;
		uavDesc.Buffer.Flags = 0;//D3D11_BUFFER_UAV_FLAG_COUNTER;// 0;//D3D11_BUFFER_UAV_FLAG_APPEND;
		hr = device.CreateUnorderedAccessView(rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mBuffer.Get(), &uavDesc, &rpc.mMoonResourceSet.mMoonVertexIndexBuffer.mUAV);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
	{
		CD3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(MoonCB);
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		hr = device.CreateBuffer(&desc, nullptr, &mCB);
		if (FAILED(hr)) LUNA_ERROR(L"");
	}
}

MoonTask::~MoonTask()
{
	mScript->release();
	mAlbedo->release();
	mNormal->release();
}

bool MoonTask::load()
{
	if (auto* script = ResourceManager::instance().load<ResourceLua>(L"data/script/mooncontroller.lua")) {
		if (!script->isValid()) {
			return false;
		}
		mScript = script;
	}
	auto& rpc = Renderer::instance().getContext();
	if (mAlbedo = ResourceManager::instance().load<TextureResource>(L"data/moon.png")) {
		if (!mAlbedo->isValid()) {
			return false;
		}
		rpc.mMoonAlbedo = &mAlbedo->getSRV();
	}
	return true;
}

void MoonTask::fixup()
{}

void MoonTask::update()
{
}

void MoonTask::fixedUpdate()
{
	if (SceneManager::instance().isPaused()) {
		return;
	}

	auto const sceneElapsed = this->getScene().getElapsedSecond();
	auto& L = mScript->getContext();

	lua_getglobal(L, "get_state");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, sceneElapsed);
	if (lua_pcall(L, 1, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	lua_pushstring(L, "posx"); 
	lua_gettable(L, -2);
	auto posx = lua_tonumber(L, -1);
	lua_pushstring(L, "posy");
	lua_gettable(L, -3);
	auto posy = lua_tonumber(L, -1);
	lua_pushstring(L, "posz");
	lua_gettable(L, -4);
	auto posz = lua_tonumber(L, -1);

	auto world = XMMatrixRotationY(sceneElapsed*0.1f) * XMMatrixTranslation(posx, posy, posz);
	XMStoreFloat4x4(&mConstant.mWorldMatrix, XMMatrixTranspose(world));
	mConstant.mVertexCount = mVertexCount;
	mDispatchFlag = true;
}

void MoonTask::draw(Renderer& renderer)
{
	auto& precomp = renderer.getPreCompute();
	precomp.draw(0, [&](RenderPassContext& rpc)
	{
		auto& dc = rpc.mContext;
		dc.UpdateSubresource(mCB.Get(), 0, nullptr, &mConstant, 0, 0);

		if (applyTechnique(dc, rpc, L"environment", "moon_transform")) {
			dc.CSSetConstantBuffers(1, 1, mCB.GetAddressOf());
			scoped_srv_cs srv(dc, 1, { mVBSRV.Get() });
			scoped_uav_cs uav(dc, 0, { rpc.mMoonResourceSet.mMoonWorldVertexBuffer.mUAV.Get() });
			dc.Dispatch((mVertexCount+255)/256, 1, 1);
		}
	});

	if(mDispatchFlag) {
		auto& ppass = renderer.getParticleSimulation();
		auto elapsed = SceneManager::instance().getElapsedSecond();
		if (elapsed < 202.f) {
			elapsed = (std::max)((std::min)(-169.f, 204.f), 0.f);
			ppass.addEmitterController([elapsed](ParticleEmitterSettings& pe) {
				pe.mEmitter.mEmitCount = 2048 * 1;
				pe.mEmitter.mEmitLifeTime = 320.f;// + elapsed * 5.f;
				pe.mEmitter.mPosition = XMFLOAT3(0.f, 1.f+elapsed*0.298f, 0.f);
				pe.mEmitter.mRadius = 0.05f;
				pe.mEmitter.mColor = XMFLOAT4(0.02f, 0.02f, 0.02f, 1.f);
				pe.mShaderName = "particle_emit_moon";
			});
		}
		mDispatchFlag = false;
	}
#if 0
	auto& solid = renderer.getSolid();
	solid.draw(1, [&](RenderPassContext& rpc)
	{
		auto& dc = rpc.mContext;

		if (applyTechnique(dc, rpc, L"environment", "moon")) {
			dc.VSSetConstantBuffers(1, 1, mCB.GetAddressOf());
			scoped_srv_ps srvps(dc, 0, { &mAlbedo->getSRV() });

			auto const stride = sizeof(Vertex);
			auto const offset = 0U;
			dc.IASetInputLayout(mIL.Get());
			dc.IASetVertexBuffers(0, 1, mVB.GetAddressOf(), &stride, &offset);
			dc.IASetIndexBuffer(mIB.Get(), DXGI_FORMAT_R32_UINT, 0);
			dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc.DrawIndexed(mIndexCount, 0, 0);
		}
	});
#endif

	auto& draw2d = renderer.getDraw2D();
	draw2d.drawText(); // TODO:
}

void MoonTask::reset()
{}

}
