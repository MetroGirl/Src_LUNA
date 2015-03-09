#pragma once
#include "lib/task.h"
#include "textureResource.h"
#include <DirectXMath.h>

namespace luna {
	using namespace DirectX;

struct MoonCB {
	XMFLOAT4X4 mWorldMatrix;
	u32 mVertexCount;
	u32 _[3];
};

	class MoonTask : public Task
	{
		LUNA_DECLARE_CONCRETE(MoonTask, Task);

	public:
		MoonTask();
		virtual ~MoonTask() override;

	private:
		bool load() override;
		void fixup() override;
		void update() override;
		void fixedUpdate() override;
		void draw(Renderer& renderer) override;
		void reset() override;

		ComPtr<ID3D11Buffer> mVB, mIB, mCB;
		ComPtr<ID3D11ShaderResourceView> mVBSRV;
		ComPtr<ID3D11InputLayout> mIL;
		TextureResource* mAlbedo, *mNormal;
		u32 mVertexCount, mIndexCount;
		MoonCB mConstant;
		ResourceLua* mScript;
		bool mDispatchFlag;
	};

}
