#include "stdafx.h"
#include "postEffectTask.h"
#include "app/renderer.h"
#include "app/renderPass/postEffectPass.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::PostEffectTask);

	PostEffectTask::PostEffectTask()
	{
	}

	PostEffectTask::~PostEffectTask()
	{
	}

	bool PostEffectTask::load()
	{
		return true;
	}

	void PostEffectTask::fixup()
	{
		auto& context = Renderer::instance().getContext();

		Renderer::instance().getPostEffect().addTask(this);
	}

	void PostEffectTask::update()
	{
		const f32 ratio = getRatio();

	}

	void PostEffectTask::fixedUpdate()
	{
		const f32 ratio = getRatio();

	}

	void PostEffectTask::draw(Renderer& renderer)
	{
		const f32 ratio = getRatio();
	}

	void PostEffectTask::onRender(const luna::TypeInfo& type, RenderPassContext& rpc, Renderer& renderer, u32& arg)
	{
		const f32 ratio = getRatio();

	}

	void PostEffectTask::reset()
	{

	}
}

