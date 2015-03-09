#pragma once
#include "lib/task.h"
#include <DirectXMath.h>

namespace luna {
	using namespace DirectX;

	class FloorTask : public Task
	{
		LUNA_DECLARE_CONCRETE(FloorTask, Task);

	public:
		FloorTask();
		virtual ~FloorTask() override;

	private:
		bool load() override;
		void fixup() override;
		void update() override;
		void fixedUpdate() override;
		void draw(Renderer& renderer) override;
		void reset() override;
	};

}
