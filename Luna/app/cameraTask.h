#pragma once
#include "lib/task.h"
#include <DirectXMath.h>
//#include "camera.h"

namespace luna {
	using namespace DirectX;
	class ResourceLua;
	class CameraTask : public Task
	{
		LUNA_DECLARE_CONCRETE(CameraTask, Task);

	public:
		CameraTask();
		virtual ~CameraTask() override;

	private:
		bool load() override;
		void fixup() override;
		void update() override;
		void fixedUpdate() override;
		void draw(Renderer& renderer) override;
		void reset() override;

		ResourceLua* mScript;
		XMFLOAT3 mPosition, mLookAt, mUp;
	};

}
