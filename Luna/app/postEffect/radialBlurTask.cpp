#include "stdafx.h"
#include "radialBlurTask.h"
#include "app/renderer.h"
#include "app/renderPass/postEffectPass.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::RadialBlurTask);

	RadialBlurTask::RadialBlurTask()
	{
	}

	RadialBlurTask::~RadialBlurTask()
	{
	}

	bool RadialBlurTask::load()
	{
		base_t::load();

		return true;
	}

	void RadialBlurTask::fixup()
	{
		base_t::fixup();

		auto& context = Renderer::instance().getContext();
	}

	void RadialBlurTask::update()
	{
		base_t::update();

		const f32 ratio = getRatio();

	}

	void RadialBlurTask::fixedUpdate()
	{
		base_t::fixedUpdate();

		const f32 ratio = getRatio();

	}

	void RadialBlurTask::draw(Renderer& renderer)
	{
		base_t::draw(renderer);

		const f32 ratio = getRatio();
	}

	void RadialBlurTask::onRender(const luna::TypeInfo& type, RenderPassContext& rpc, Renderer& renderer, u32& arg)
	{
		base_t::onRender(type, rpc, renderer, arg);

		const f32 ratio = getRatio();

		{
			auto techniquePtr = rpc.mShaderMap[L"posteffect"]->findTechnique("radialBlur");

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
		}
	}

	void RadialBlurTask::reset()
	{
		base_t::reset();
	}
}

