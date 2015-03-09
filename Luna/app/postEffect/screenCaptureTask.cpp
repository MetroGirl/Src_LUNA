#include "stdafx.h"
#include "screenCaptureTask.h"
#include "app/renderer.h"
#include "app/renderPass/postEffectPass.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::ScreenCaptureTask);

	ScreenCaptureTask::ScreenCaptureTask()
		: mResourcePtr(nullptr)
	{
	}

	ScreenCaptureTask::~ScreenCaptureTask()
	{
	}

	bool ScreenCaptureTask::load()
	{
		if (!mResourcePtr){
			mResourcePtr = ResourceManager::instance().load<ResourceLua>((wstring(L"data/script/screenCapture/") + getScene().getName() + wstring(L".lua")).c_str());
		}
		if (!mResourcePtr){
			return true;// たぶんリソースがない。終わったことにしておく。
		}
		if (mResourcePtr && mResourcePtr->isValid()){
			return true;
		}

		return false;
	}

	void ScreenCaptureTask::fixup()
	{
		updateEffectTbl();
	}

	void ScreenCaptureTask::update()
	{
		const f32 ratio = getRatio();

		if (!mResourcePtr){
			return ;
		}
		if (mResourcePtr->isReloaded())
		{
			updateEffectTbl();
		}

		auto& rpc = Renderer::instance().getContext();
		for (auto& effect : mEffectTbl){
			const f32 sceneRatio = getScene().getCurrentFrameRatio();
			if (effect.consumed){
				continue;
			}
			if (effect.timing > ratio){
				continue;
			}
			effect.consumed = true;

			auto& dc = rpc.mContext;
			{
				ID3D11RenderTargetView* rtvTbl[] = { rpc.mFrameBufferTbl[FrameBufferType_CaptureTexture0 + effect.slot].getColorRTV().Get() };
				GraphicsDevice::instance().getDeviceContext().OMSetRenderTargets(1, rtvTbl, nullptr);

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
				dc.PSSetShaderResources(0, _countof(srvTbl), srvTbl);

				auto techniquePtr = rpc.mShaderMap[L"posteffect"]->findTechnique(effect.technique.c_str());

				auto& context = rpc.mContext;
				context.IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
				context.IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
				context.IASetInputLayout(nullptr);
				context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				context.VSSetShader(techniquePtr->mVS.shaderPtr, nullptr, 0);
				context.HSSetShader(techniquePtr->mHS.shaderPtr, nullptr, 0);
				context.DSSetShader(techniquePtr->mDS.shaderPtr, nullptr, 0);
				context.GSSetShader(techniquePtr->mGS.shaderPtr, nullptr, 0);
				context.CSSetShader(techniquePtr->mCS.shaderPtr, nullptr, 0);
				context.PSSetShader(techniquePtr->mPS.shaderPtr, nullptr, 0);

				context.Draw(3, 0);

				context.VSSetShader(nullptr, nullptr, 0);
				context.HSSetShader(nullptr, nullptr, 0);
				context.DSSetShader(nullptr, nullptr, 0);
				context.GSSetShader(nullptr, nullptr, 0);
				context.CSSetShader(nullptr, nullptr, 0);
				context.PSSetShader(nullptr, nullptr, 0);

				memset(srvTbl, 0, sizeof(srvTbl));
				dc.PSSetShaderResources(0, _countof(srvTbl), srvTbl);
			}
		}
	}

	void ScreenCaptureTask::fixedUpdate()
	{
		const f32 ratio = getRatio();

	}

	void ScreenCaptureTask::draw(Renderer& renderer)
	{
		const f32 ratio = getRatio();
	}

	void ScreenCaptureTask::onRender(const luna::TypeInfo& type, RenderPassContext& rpc, Renderer& renderer, u32& arg)
	{
	}

	void ScreenCaptureTask::reset()
	{
	}

	void ScreenCaptureTask::updateEffectTbl()
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

			f32 timing = 0;
			u32 slot = 0;
			const c8* technique = "copyAsIs";

			while (lua_next(L, -2)){
				auto const* a = lua_tostring(L, -2);
				if (strcmp(a, "Timing") == 0){
					timing = (f32)lua_tonumber(L, -1);
				}
				else if (strcmp(a, "Slot") == 0){
					slot = (u32)lua_tonumber(L, -1);
				}
				lua_pop(L, 1);
			}

			EffectInfo info;
			info.timing = timing;
			info.slot = slot;
			info.consumed = false;
			info.technique = technique;
			mEffectTbl.push_back(info);

			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
}

