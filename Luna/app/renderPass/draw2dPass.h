#pragma once

#include "app/renderPass.h"
#include "lib/gfx/deviceDX11.h"

namespace luna {

class Draw2DPass : public RenderPass
{
	LUNA_DECLARE_ABSTRACT(Draw2DPass, RenderPass);
public:
	Draw2DPass(RenderPassContext& rpc);

	void drawText();
	void drawImage();
	void drawRectangle(s32 x, s32 y, s32 width, s32 height, u32 color);

private:
	void preSettings(RenderPassContext& rpc) override;
	void render(RenderPassContext& rpc) const override;
	void postSettings(RenderPassContext& rpc) override;

	enum class BatchType
	{
		Line,
		Triangle,
	};
	struct Batch
	{
		BatchType mType;
		u8 mVertexCount;
		u8 mIndexCount;
	};
	struct Vertex
	{
		float x, y, z, w;
		u32 color;
	};
	std::vector<Batch> mBatch;
	std::vector<Vertex> mVertexBuffer;
	std::vector<u16> mIndexBuffer;

	ComPtr<ID3D11Buffer> mVB;
	ComPtr<ID3D11Buffer> mIB;
	ComPtr<ID3D11InputLayout> mIL;
};

}
