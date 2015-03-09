#include "stdafx.h"
#include "lib/sceneManager.h"
#include "app/renderer.h"
#include "app/renderPass/draw2dPass.h"
#include "app/renderPass/fluidPass.h"
#include "app/renderPass/particlePass.h"
#include "app/renderPass/postEffectPass.h"

namespace luna {
namespace {

void createRenderPasses(vector<unique_ptr<RenderPass>>& passes, RenderPassContext& rpc)
{
	passes.push_back(make_unique<PreComputePass>(rpc));
	passes.push_back(make_unique<FluidPass>(rpc));
	passes.push_back(make_unique<ParticleSimulationPass>(rpc));
	passes.push_back(make_unique<ParticleShadowPass>(rpc));
	passes.push_back(make_unique<SolidObjectPass>(rpc));
	passes.push_back(make_unique<FluidRenderPass>(rpc));
	passes.push_back(make_unique<ParticleRenderPass>(rpc));
	passes.push_back(make_unique<PostEffectPass>(rpc));
	passes.push_back(make_unique<Draw2DPass>(rpc));
	passes.push_back(make_unique<DevelopDrawPass>(rpc));
}

ComPtr<ID3D11Buffer> createSceneCB(ID3D11Device& device)
{
	ComPtr<ID3D11Buffer> buf;
	D3D11_BUFFER_DESC desc;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.ByteWidth = sizeof(SceneCB);
	desc.CPUAccessFlags = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	auto hr = device.CreateBuffer(&desc, nullptr, &buf);
	if (FAILED(hr)){
		LUNA_ERROR(L"ID3D11Device::CreateBuffer");
		LUNA_ASSERT(0, L"");
	}
	return buf;
}

void createFrameBuffers(array<FrameBuffer, FrameBufferType_Num>& frameBufferTbl)
{
	const s32 width = SettingsManager::instance().getWidth();
	const s32 height = SettingsManager::instance().getHeight();

	for (s32 i = FrameBufferType_SceneTextureBegin; i < FrameBufferType_SceneTextureEnd; ++i){
		auto& slot = frameBufferTbl[i];
		slot.consturct(width, height, FrameBuffer::ScreenBufferType_ColorAndDepth, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}
	for (s32 i = FrameBufferType_WorkBufferBegin; i < FrameBufferType_WorkBufferEnd; ++i){
		auto& slot = frameBufferTbl[i];
		slot.consturct(width, height, FrameBuffer::ScreenBufferType_ColorOnly, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}
	for (s32 i = FrameBufferType_CaptureTextureBegin; i < FrameBufferType_CaptureTextureEnd; ++i){
		auto& slot = frameBufferTbl[i];
		slot.consturct(width, height, FrameBuffer::ScreenBufferType_ColorOnly, DXGI_FORMAT_R16G16B16A16_FLOAT);
	}
}

void createSamplers(array<SamplerState, SamplerStateType_Num>& samplerStateTbl)
{
	for (s32 i = SamplerStateType_Begin; i < SamplerStateType_End; ++i){
		auto& slot = samplerStateTbl[i];
		slot.consturct((SamplerStateType)i);
	}
}

void createBlendStates(array<BlendState, BlendStateType_Num>& blendStateTbl)
{
	for (s32 i = BlendStateType_Begin; i < BlendStateType_End; ++i){
		auto& slot = blendStateTbl[i];
		slot.consturct((BlendStateType)i);
	}
}

template <class T>
T& findPass(std::vector<unique_ptr<RenderPass>> const& passes)
{
	auto it = std::find_if(begin(passes), end(passes), [](std::unique_ptr<RenderPass> const& rp) {
		return rp->isA<T>();
	});
	if (it == end(passes)) LUNA_ERROR(L"Pass not found: .");
	return *static_cast<T*>(it->get());
}

} // unnamed

Renderer::Renderer(ID3D11Device& device, ID3D11DeviceContext& context, std::unordered_map<std::wstring, ShaderResource*>&& map, u32 width, u32 height)
	: mRPC(device, context, std::move(map), mFrameBuffers, mBlendStates, width, height)
	, mSceneCB(createSceneCB(device))
{
	mInstancePtr = this;

	createRenderPasses(mPasses, mRPC);
	createFrameBuffers(mFrameBuffers);
	createSamplers(mSamplers);
	createBlendStates(mBlendStates);

	mScene.mResolution = XMFLOAT2((f32)SettingsManager::instance().getWidth(), (f32)SettingsManager::instance().getHeight());
	mScene.mResolutionInv = XMFLOAT2(1.f / mScene.mResolution.x, 1.f / mScene.mResolution.y);
}

void Renderer::setMainCamera(XMFLOAT3 const& position, XMFLOAT3 const& at, XMFLOAT3 const& up)
{
	auto camPos = XMLoadFloat3(&position);
	auto camAt = XMLoadFloat3(&at);
	auto camUp = XMLoadFloat3(&up);
	auto viewMat = XMMatrixLookAtLH(camPos, camAt, camUp);
	auto viewInvMat = XMMatrixInverse(nullptr, viewMat);
	auto projMat = XMMatrixPerspectiveFovLH(mRPC.mFovY, static_cast<f32>(mRPC.mWindowWidth) / mRPC.mWindowHeight, mRPC.mNearZ, mRPC.mFarZ);
	XMStoreFloat4x4(&mScene.mViewMatrix, XMMatrixTranspose(viewMat));
	XMStoreFloat4x4(&mScene.mProjectionMatrix, XMMatrixTranspose(projMat));
	XMStoreFloat4x4(&mScene.mViewProjectionMatrix, XMMatrixTranspose(viewMat * projMat));
	XMStoreFloat4x4(&mScene.mViewInverseMatrix, XMMatrixTranspose(viewInvMat));
	mScene.mEyePosition = XMFLOAT4(position.x, position.y, position.z, 0.f);
}

Draw2DPass& Renderer::getDraw2D() const
{
	auto it = std::find_if(begin(mPasses), end(mPasses), [](std::unique_ptr<RenderPass> const& rp) {
		return rp->isA<Draw2DPass>();
	});
	if (it == end(mPasses)) LUNA_ERROR(L"Draw2DPass not found.");
	return *static_cast<Draw2DPass*>(it->get());
}

PreComputePass& Renderer::getPreCompute() const
{
	return findPass<PreComputePass>(mPasses);
}

SolidObjectPass& Renderer::getSolid() const
{
	return findPass<SolidObjectPass>(mPasses);
}

ParticleSimulationPass& Renderer::getParticleSimulation() const
{
	return findPass<ParticleSimulationPass>(mPasses);
}

ParticleRenderPass& Renderer::getParticleRender() const
{
	return findPass<ParticleRenderPass>(mPasses);
}

PostEffectPass& Renderer::getPostEffect() const
{
	return findPass<PostEffectPass>(mPasses);
}

FluidPass& Renderer::getFluid() const
{
	return findPass<FluidPass>(mPasses);
}

FluidRenderPass& Renderer::getFluidRender() const
{
	return findPass<FluidRenderPass>(mPasses);
}

void Renderer::execute()
{
	auto& dc = mRPC.mContext;

	setRenderTarget(SceneLayer_0);

	// フレームに一度だけ更新するバッファなどはここで更新・設定
	updateSystemCBuffer();

	dc.UpdateSubresource(mSceneCB.Get(), 0, nullptr, &mScene, 0, 0);
	dc.VSSetConstantBuffers(0, 1, mSceneCB.GetAddressOf());
	dc.HSSetConstantBuffers(0, 1, mSceneCB.GetAddressOf());
	dc.DSSetConstantBuffers(0, 1, mSceneCB.GetAddressOf());
	dc.GSSetConstantBuffers(0, 1, mSceneCB.GetAddressOf());
	dc.PSSetConstantBuffers(0, 1, mSceneCB.GetAddressOf());
	dc.CSSetConstantBuffers(0, 1, mSceneCB.GetAddressOf());
	{
		ID3D11SamplerState* samplers[] = {
			mSamplers[SamplerStateType_Nearest_Clamp].getSampler().Get(),
			mSamplers[SamplerStateType_Nearest_Repeat].getSampler().Get(),
			mSamplers[SamplerStateType_Nearest_Mirror].getSampler().Get(),
			mSamplers[SamplerStateType_Bilinear_Clamp].getSampler().Get(),
			mSamplers[SamplerStateType_Bilinear_Repeat].getSampler().Get(),
			mSamplers[SamplerStateType_Bilinear_Mirror].getSampler().Get(),
			mSamplers[SamplerStateType_Trilinear_Clamp].getSampler().Get(),
			mSamplers[SamplerStateType_Trilinear_Repeat].getSampler().Get(),
			mSamplers[SamplerStateType_Trilinear_Mirror].getSampler().Get(),
			mSamplers[SamplerStateType_Anisotropic_Clamp].getSampler().Get(),
			mSamplers[SamplerStateType_Anisotropic_Repeat].getSampler().Get(),
			mSamplers[SamplerStateType_Anisotropic_Mirror].getSampler().Get(),
		};
		dc.VSSetSamplers(4, ARRAYSIZE(samplers), samplers);
		dc.HSSetSamplers(4, ARRAYSIZE(samplers), samplers);
		dc.DSSetSamplers(4, ARRAYSIZE(samplers), samplers);
		dc.GSSetSamplers(4, ARRAYSIZE(samplers), samplers);
		dc.PSSetSamplers(4, ARRAYSIZE(samplers), samplers);
		dc.CSSetSamplers(4, ARRAYSIZE(samplers), samplers);
	}

	for (auto const& pass : mPasses)
	{
		pass->preSettings(mRPC);
		pass->render(mRPC);
		pass->postSettings(mRPC);
	}

	// ここより後に発行された描画コマンドは画面に出力されない
}

void Renderer::setRenderTarget(SceneLayer layer)
{
	auto& fbo = mFrameBuffers[FrameBufferType_SceneTexture0 + layer];
	ID3D11RenderTargetView* rtvTbl[] = { fbo.getColorRTV().Get() };
	GraphicsDevice::instance().getDeviceContext().OMSetRenderTargets(1, rtvTbl, fbo.getDepthDSV().Get());

	f32 color[] = { 0.2f, 0.2f, 0.8f, 1.f };
	auto& context = getContext().mContext;
	context.ClearRenderTargetView(fbo.getColorRTV().Get(), color);
	context.ClearDepthStencilView(fbo.getDepthDSV().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
}

void Renderer::updateSystemCBuffer()
{
	mScene.mSettings = XMFLOAT4(SettingsManager::instance().isBigScreenMode() ? 1.f : 0.f, 0,0,0);
	mScene.mAspectV = SettingsManager::instance().getAspectV();
	mScene.mAspectH = SettingsManager::instance().getAspectH();

	mScene.mDemoTimeSecond = SceneManager::instance().getElapsedSecond();
	mScene.mDemoTimeRatio = mScene.mDemoTimeSecond < FLT_EPSILON ? 0.0f : SceneManager::instance().getDuration() / mScene.mDemoTimeSecond;

	const auto& sceneElapsedTbl = SceneManager::instance().getSceneDuration();
	for (size_t i = 0; i < sceneElapsedTbl.size(); ++i){
		mScene.mSceneTimeSecond[i] = XMFLOAT4(sceneElapsedTbl[i], sceneElapsedTbl[i], sceneElapsedTbl[i], sceneElapsedTbl[i]);
	}

	const auto& sceneRatioTbl = SceneManager::instance().getSceneRatio();
	for (size_t i = 0; i < sceneRatioTbl.size(); ++i){
		mScene.mSceneTimeRatio[i] = XMFLOAT4(sceneRatioTbl[i], sceneRatioTbl[i], sceneRatioTbl[i], sceneRatioTbl[i]);
	}

	{// FFT結果をバンドごとに分割
		const s32 FFTSample = 1024;
		f32 fft[FFTSample] = {};
		SoundManager::instance().getFFTMagnitude(fft);
		const s32 BandNum = _countof(mScene.mSoundFFTBand);

		s32 b0 = 0;
		for (s32 x = 0; x < BandNum; x++) {
			f32 peak = 0;
			s32 b1 = (s32)pow(2, x*10.0 / (BandNum - 1));
			if (b1 <= b0){
				b1 = b0 + 1;
			}
			if (b1 > FFTSample - 1){
				b1 = FFTSample - 1;
			}
			for (; b0 < b1; b0++){
				if (peak < fft[1 + b0]){
					peak = fft[1 + b0];
				}
			}
			const f32 v = sqrt(peak) * 3.0f;// sqrtで底上げ
			mScene.mSoundFFTBand[x] = XMFLOAT4(v, v, v, v);
		}
		{
			const f32 baseFreq = 44100.f;

			f32 low = 0.f;
			f32 mid = 0.f;
			f32 high = 0.f;

			u32 lowband = (u32)(floor((400.0 * f32(FFTSample)) / baseFreq) + 0.5f);
			u32 midband = (u32)(floor((1760.0 * f32(FFTSample)) / baseFreq) + 0.5f);
			u32 highband = FFTSample;// 1760Hz～

			u32 count_lowband = lowband - 0;
			u32 count_midband = midband - lowband;
			u32 count_highband = highband - midband;

			for (u32 i = 0; i < lowband; ++i){
				low += fft[i];
			}
			for (u32 i = lowband; i < midband; ++i){
				mid += fft[i];
			}
			for (u32 i = midband; i < highband; ++i){
				high += fft[i];
			}

			low /= (f32)count_lowband;
			mid /= (f32)count_midband;
			high /= (f32)count_highband;

			mScene.mSoundFFTAvg = XMFLOAT4(low, mid, high, 0);
		}
#if !LUNA_PUBLISH
		{
			auto& draw2d = this->getDraw2D();
			int x = mRPC.mWindowWidth - 220;
			int y = mRPC.mWindowHeight - 20;
			for (auto i = 0; i < BandNum; ++i) {
				auto height = static_cast<int>(mScene.mSoundFFTBand[i].x * 500.f);
				auto top = y - height;
				draw2d.drawRectangle(x, top, 200 / BandNum, height, 0xffff0000);
				x += 200 / BandNum;
			}
			x = mRPC.mWindowWidth - 500;
			y = mRPC.mWindowHeight - 20;

			draw2d.drawRectangle(2, mRPC.mWindowHeight - 16, mRPC.mWindowWidth - 4, 6, 0xff222222);
			draw2d.drawRectangle(2, mRPC.mWindowHeight - 16, (mRPC.mWindowWidth - 4)*SceneManager::instance().getSceneRatio()[0], 6, 0xffff0000);
			auto duration = SceneManager::instance().getDuration();
			auto elapsed = SceneManager::instance().getElapsedSecond();
			draw2d.drawRectangle(2, mRPC.mWindowHeight - 8, mRPC.mWindowWidth - 4, 6, 0xff222222);
			draw2d.drawRectangle(2, mRPC.mWindowHeight - 8, (mRPC.mWindowWidth - 4)*(elapsed / duration), 6, 0xffff0000);
		}
#endif
	}
}

}

namespace luna{
	LUNA_IMPLEMENT_SINGLETON(luna::Renderer);
	LUNA_IMPLEMENT_ABSTRACT(luna::ObjectPass);

	LUNA_IMPLEMENT_ABSTRACT(luna::PreComputePass);
	LUNA_IMPLEMENT_ABSTRACT(luna::RenderPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::FluidPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::ParticleSimulationPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::ParticleShadowPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::SolidObjectPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::FluidRenderPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::ParticleRenderPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::PostEffectPass);
	LUNA_IMPLEMENT_ABSTRACT(luna::DevelopDrawPass);
}
