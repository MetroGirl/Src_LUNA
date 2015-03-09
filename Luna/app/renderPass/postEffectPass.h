#pragma once
#include "app/renderPass.h"
#include "app/textureResource.h"
#include "app/renderPass/context.h"
#include <DirectXMath.h>
#include <functional>

namespace luna {
	class Task;

	class PostEffectPass : public RenderPass
	{
		LUNA_DECLARE_ABSTRACT(PostEffectPass, RenderPass);
	public:
		PostEffectPass(RenderPassContext& rpc){}
		void render(RenderPassContext& rpc) const;

		void addTask(Task* taskPtr)
		{
			mTaskTbl.push_back(taskPtr);
		}

	private:
		vector<Task*> mTaskTbl;
	};

	class DevelopDrawPass : public RenderPass
	{
		LUNA_DECLARE_ABSTRACT(DevelopDrawPass, RenderPass);
	public:
		DevelopDrawPass(RenderPassContext& rpc){}

		void render(RenderPassContext& rpc) const;
	};
}
