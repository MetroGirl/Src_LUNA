#pragma once
#include "lib/task.h"
#include <DirectXMath.h>

namespace luna {
	using namespace DirectX;
	class ResourceLua;
	struct FluidGrid;
	struct PressureSource;
	class FluidGridTask : public Task
	{
		LUNA_DECLARE_CONCRETE(FluidGridTask, Task);

	public:
		FluidGridTask();
		virtual ~FluidGridTask() override;

	private:
		bool load() override;
		void fixup() override;
		void update() override;
		void fixedUpdate() override;
		void draw(Renderer& renderer) override;
		void reset() override;

		ResourceLua* mScript;
		std::shared_ptr<FluidGrid> mGrid;
		std::vector<PressureSource> mPressureSources;
		bool mReset;

		bool mDispatchFlag;
	};

}
