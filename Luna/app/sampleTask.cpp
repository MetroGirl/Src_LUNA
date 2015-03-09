#include "stdafx.h"
#include "sampleTask.h"
#include "renderer.h"
#include "app/renderPass/draw2DPass.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::SampleTask);

	SampleTask::SampleTask()
		: mShaderPtr(nullptr)
	{
	}

	SampleTask::~SampleTask()
	{
	}

	bool SampleTask::load()
	{
		if (!mShaderPtr){
			mShaderPtr = ResourceManager::instance().load<ShaderResource>(L"data/shader/sampleTask.lua");
		}
		if (!mShaderPtr->isValid()){
			return false;
		}

		return true;
	}

	void SampleTask::fixup()
	{
		// バッファ作成
		auto& context = Renderer::instance().getContext();
	}

	void SampleTask::update()
	{
		const f32 ratio = getRatio();

	}

	void SampleTask::fixedUpdate()
	{
		const f32 ratio = getRatio();

	}

	void SampleTask::draw(Renderer& renderer)
	{
		const f32 ratio = getRatio();

		auto techniquePtr = mShaderPtr->findTechnique("default");//TODO cache.

		auto& context = renderer.getContext().mContext;
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

#if !LUNA_PUBLISH
		// test
		auto& draw2d = renderer.getDraw2D();
		draw2d.drawRectangle(20, 20, 20, 20, 0xffff0000);
#endif
	}

	void SampleTask::reset()
	{

	}
}

