#pragma once
#include "lib/task.h"
#include <DirectXMath.h>

namespace luna {
	using namespace DirectX;

	struct EmitterAttribute
	{
		XMFLOAT3 mEmitterPosition;
		f32 mEmitterRadius;
		u32 mEmitCount;
		f32 mEmitLifeTime;
		u32 mUniversalIndex;
	};

	class ParticleEmitterTask : public Task
	{
		LUNA_DECLARE_CONCRETE(ParticleEmitterTask, Task);

	public:
		ParticleEmitterTask();
		virtual ~ParticleEmitterTask() override;

	private:
		bool load() override;
		void fixup() override;
		void update() override;
		void fixedUpdate() override;
		void draw(Renderer& renderer) override;
		void reset() override;
		void triggerUpdate(f32 elapsed) override;

		std::string mShaderName;
		std::string mRenderShaderName;
		ResourceLua* mScript;
		std::vector<EmitterAttribute> mEmitters;
		bool mDispatchFlag;
	};

}
