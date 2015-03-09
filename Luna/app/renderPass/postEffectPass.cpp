#include "stdafx.h"
#include "app/renderPass/postEffectPass.h"
#include "app/renderPass/context.h"
#include "app/shaderResource.h"
#include "app/renderer.h"
#include "lib/gfx/utilityDX11.h"

namespace luna {
	void PostEffectPass::render(RenderPassContext& rpc) const
	{
		auto& dc = rpc.mContext;

		// シーンバッファのフルコピー...
		// -- これを通過すると、WorkBuffer1にSceneTexture0の内容が書き込まれる
		{
			ID3D11RenderTargetView* rtvTbl[] = { rpc.mFrameBufferTbl[FrameBufferType_WorkBuffer1].getColorRTV().Get() };
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

			auto techniquePtr = rpc.mShaderMap[L"posteffect"]->findTechnique("copyAsIs");

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

		// ポストエフェクト
		u32 writeBufferPt = 0;
		{
			size_t ranTaskCount = 0;
 			for (auto& taskPtr : mTaskTbl){
 				if (taskPtr->isRunnable()){
 					taskPtr->onRender(PostEffectPass::TypeInfo, rpc, Renderer::instance(), writeBufferPt);
 					++ranTaskCount;
 				}
 			}
			if (ranTaskCount == 0){
				ID3D11RenderTargetView* rtvTbl[] = { rpc.mFrameBufferTbl[FrameBufferType_WorkBuffer0 + writeBufferPt].getColorRTV().Get() };
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

				{
					ID3D11ShaderResourceView* srvTbl[] =
					{
						rpc.mFrameBufferTbl[FrameBufferType_WorkBuffer1].getColorSRV().Get()
					};
					dc.PSSetShaderResources(0, 1, srvTbl);
				}

				// ひとつもポストエフェクトが走ってない場合は、WorkBuffer1をPostEffectへ転送する
				auto techniquePtr = rpc.mShaderMap[L"posteffect"]->findTechnique("copyAsIs");

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

		// バックバッファへ転送
		{
			ID3D11RenderTargetView* rtvTbl[] = { &GraphicsDevice::instance().getBackBuffer() };
			GraphicsDevice::instance().getDeviceContext().OMSetRenderTargets(1, rtvTbl, &GraphicsDevice::instance().getDepthStencil());

			auto techniquePtr = rpc.mShaderMap[L"posteffect"]->findTechnique("finalCombiner");

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

			ID3D11ShaderResourceView* srvTbl[] = { rpc.mFrameBufferTbl[FrameBufferType_WorkBuffer0 + writeBufferPt^1].getColorSRV().Get() };
			context.PSSetShaderResources(0, 1, srvTbl);

			context.Draw(3, 0);

			{
				ID3D11ShaderResourceView* srvTbl[] = { nullptr };
				context.PSSetShaderResources(0, 1, srvTbl);
			}

			context.VSSetShader(nullptr, nullptr, 0);
			context.HSSetShader(nullptr, nullptr, 0);
			context.DSSetShader(nullptr, nullptr, 0);
			context.GSSetShader(nullptr, nullptr, 0);
			context.CSSetShader(nullptr, nullptr, 0);
			context.PSSetShader(nullptr, nullptr, 0);
		}
	}

	void DevelopDrawPass::render(RenderPassContext& rpc) const
	{
		auto& dc = rpc.mContext;
	}
}
