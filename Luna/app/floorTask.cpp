#include "stdafx.h"
#include "floorTask.h"
#include "renderer.h"
#include "renderPass/particlePass.h"
#include "renderPass/draw2dPass.h"
#include "stringHelper.h"
#include "lib/gfx/utilityDX11.h"

namespace luna {

LUNA_IMPLEMENT_CONCRETE(luna::FloorTask);

FloorTask::FloorTask()
{}

FloorTask::~FloorTask()
{
}

bool FloorTask::load()
{
	return true;
}

void FloorTask::fixup()
{}

void FloorTask::update()
{
}

void FloorTask::fixedUpdate()
{
	if (SceneManager::instance().isPaused()) {
		return;
	}
}

void FloorTask::draw(Renderer& renderer)
{
	auto& solid = renderer.getSolid();
	solid.draw(0, [](RenderPassContext& rpc)
	{
		auto& dc = rpc.mContext;
		if (applyTechnique(dc, rpc, L"environment", "draw_floor"))
		{
			scoped_srv_ps srvps(dc, 0, { rpc.mParticleShadowResourceSet.mDepthSRV.Get() });
			dc.IASetInputLayout(nullptr);
			dc.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			dc.Draw(3, 0);
		}
	});

	auto& draw2d = renderer.getDraw2D();
	draw2d.drawText(); // TODO:
}

void FloorTask::reset()
{}

}
