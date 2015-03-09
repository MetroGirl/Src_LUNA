#include "stdafx.h"
#include "genericPostEffectTask.h"
#include "app/renderer.h"
#include "app/renderPass/postEffectPass.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::GenericPostEffectTask);

	GenericPostEffectTask::GenericPostEffectTask()
		: mResourcePtr(nullptr)
	{
	}

	GenericPostEffectTask::~GenericPostEffectTask()
	{
	}

	bool GenericPostEffectTask::load()
	{
		base_t::load();

		if (!mResourcePtr){
			mResourcePtr = ResourceManager::instance().load<ResourceLua>((wstring(L"data/script/posteffect/") + getScene().getName() + wstring(L".lua")).c_str());
		}
		if (!mResourcePtr){
			return true;// たぶんリソースがない。終わったことにしておく。
		}
		if (mResourcePtr && mResourcePtr->isValid()){
			return true;
		}

		return false;
	}

	void GenericPostEffectTask::fixup()
	{
		base_t::fixup();

		auto& context = Renderer::instance().getContext();

		D3D11_BUFFER_DESC desc;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.ByteWidth = sizeof(EffectCB);
		desc.CPUAccessFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		auto hr = context.mDevice.CreateBuffer(&desc, nullptr, mEffectCB.GetAddressOf());
		if (FAILED(hr)){
			LUNA_ERROR(L"ID3D11Device::CreateBuffer");
			LUNA_ASSERT(0, L"");
		}

		updateEffectTbl();
	}

	void GenericPostEffectTask::update()
	{
		base_t::update();

		const f32 ratio = getRatio();

		if (!mResourcePtr){
			return ;
		}
		if (mResourcePtr->isReloaded())
		{
			updateEffectTbl();
		}
	}

	void GenericPostEffectTask::fixedUpdate()
	{
		base_t::fixedUpdate();

		const f32 ratio = getRatio();

	}

	void GenericPostEffectTask::draw(Renderer& renderer)
	{
		base_t::draw(renderer);

		const f32 ratio = getRatio();
	}

	void GenericPostEffectTask::onRender(const luna::TypeInfo& type, RenderPassContext& rpc, Renderer& renderer, u32& arg)
	{
		base_t::onRender(type, rpc, renderer, arg);

		const f32 ratio = getRatio();

		for (auto& effect : mEffectTbl){
			const f32 sceneRatio = getScene().getCurrentFrameRatio();
			if (!(effect.start <= sceneRatio && sceneRatio <= effect.end))
			{
				continue;
			}
			// const f32 effectRatio = (1.f / (effect.end - effect.start) * sceneRatio) - effect.start;
			const f32 effectRatio = (sceneRatio - effect.start) * (1.f / (effect.end - effect.start));

			auto techniquePtr = rpc.mShaderMap[L"posteffect"]->findTechnique(effect.techniqueName.c_str());
			if (!techniquePtr){
				continue;
			}

			auto& context = rpc.mContext;
			{
				ID3D11RenderTargetView* rtvTbl[] = { rpc.mFrameBufferTbl[FrameBufferType_WorkBuffer0 + arg].getColorRTV().Get() };
				GraphicsDevice::instance().getDeviceContext().OMSetRenderTargets(1, rtvTbl, nullptr);
			}

			ID3D11ShaderResourceView* srvTbl[] =
			{
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture0].getColorSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture1].getColorSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture2].getColorSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture3].getColorSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture0].getDepthSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture1].getDepthSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture2].getDepthSRV().Get(),
				rpc.mFrameBufferTbl[FrameBufferType_SceneTexture3].getDepthSRV().Get(),
			};
			context.PSSetShaderResources(0, _countof(srvTbl), srvTbl);

			{
				ID3D11ShaderResourceView* srvTbl[] =
				{
					rpc.mFrameBufferTbl[FrameBufferType_WorkBuffer0 + arg ^ 1].getColorSRV().Get()
				};
				context.PSSetShaderResources(0, 1, srvTbl);
			}

			context.IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
			context.IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
			context.IASetInputLayout(nullptr);
			context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			mEffect.sceneRatio = XMFLOAT4(sceneRatio, 0, 0, 0);
			mEffect.effectRatio = XMFLOAT4(effectRatio, 0, 0, 0);
			context.UpdateSubresource(mEffectCB.Get(), 0, nullptr, &mEffect, 0, 0);
			ID3D11Buffer* cbTbl[]{ mEffectCB.Get() };
			context.PSSetConstantBuffers(2, 1, cbTbl);

			for (auto& v : effect.patchTbl){
				if (v.isResource){
					if (v.resourcePtr){
						ID3D11ShaderResourceView* svTbl[]{ &v.resourcePtr->getSRV() };
						context.PSSetShaderResources(v.registerSlot, 1, svTbl);
					}
					else{
						ID3D11ShaderResourceView* svTbl[]{ nullptr };
						context.PSSetShaderResources(v.registerSlot, 1, svTbl);
					}
				}
				else{
					ID3D11ShaderResourceView* svTbl[]{ 
						rpc.mFrameBufferTbl[FrameBufferType_CaptureTexture0 + v.fbSlot].getColorSRV().Get()
					};
					context.PSSetShaderResources(v.registerSlot, 1, svTbl);
				}
			}

			context.VSSetShader(techniquePtr->mVS.shaderPtr, nullptr, 0);
			context.HSSetShader(techniquePtr->mHS.shaderPtr, nullptr, 0);
			context.DSSetShader(techniquePtr->mDS.shaderPtr, nullptr, 0);
			context.GSSetShader(techniquePtr->mGS.shaderPtr, nullptr, 0);
			context.CSSetShader(techniquePtr->mCS.shaderPtr, nullptr, 0);
			context.PSSetShader(techniquePtr->mPS.shaderPtr, nullptr, 0);

			context.Draw(3, 0);

			arg ^= 1;

			memset(cbTbl, 0, sizeof(cbTbl));
			context.PSSetConstantBuffers(2, 1, cbTbl);

			memset(srvTbl, 0, sizeof(srvTbl));
			context.PSSetShaderResources(0, _countof(srvTbl), srvTbl);
		}
	}

	void GenericPostEffectTask::reset()
	{
		base_t::reset();
	}

	void GenericPostEffectTask::updateEffectTbl()
	{
		if (!mResourcePtr){
			return;
		}
		mEffectTbl.clear();
		auto& L = mResourcePtr->getContext();

		lua_getglobal(L, "Entries");
		lua_pushnil(L);
		while (lua_next(L, -2)) {
			lua_pushnil(L);

			f32 start = 0;
			f32 end = 1;
			const c8* technique = "";
			vector<pair<string, string>> patchTbl;
			XMFLOAT4 metadata1, metadata2;
			
			while (lua_next(L, -2)){
				auto const* a = lua_tostring(L, -2);
				if (strcmp(a, "Start") == 0){
					start = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "End") == 0){
					end = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "Technique") == 0){
					technique = lua_tostring(L, -1);
				}
				else if (a[0] == 't'){
					patchTbl.push_back(pair<string, string>(a, lua_tostring(L, -1)));
				}
				else if (strcmp(a, "m1_x")==0){
					metadata1.x = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m1_y") == 0){
					metadata1.y = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m1_z") == 0){
					metadata1.z = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m1_w") == 0){
					metadata1.w = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m2_x") == 0){
					metadata2.x = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m2_y") == 0){
					metadata2.y = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m2_z") == 0){
					metadata2.z = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "m2_w") == 0){
					metadata2.w = (f32)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);
			}

			EffectInfo info;
			info.start = start;
			info.end = end;
			info.techniqueName = technique;
			for (auto& v : patchTbl){
				PatchInfo pinfo;
				pinfo.registerSlot = atol(v.first.c_str() + 1);
				if (!v.second.empty() && v.second[0] == 'c'){
					pinfo.isResource = false;
					pinfo.fbSlot = atol(v.second.c_str());
				}
				else{
					pinfo.isResource = true;
					pinfo.resourcePtr = ResourceManager::instance().load<TextureResource>(v.second.c_str());
				}
				info.patchTbl.push_back(pinfo);
			}
			info.metadata1 = metadata1;
			info.metadata2 = metadata2;
			mEffectTbl.push_back(info);

			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
}

