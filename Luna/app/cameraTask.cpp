#include "stdafx.h"
#include "cameraTask.h"
#include "renderer.h"
#include "renderPass/draw2dPass.h"
#include "lib/resource/resourceLua.h"
#include "stringHelper.h"

namespace luna {

LUNA_IMPLEMENT_CONCRETE(luna::CameraTask);

CameraTask::CameraTask()
	: mScript(nullptr)
	, mPosition(2.f, 2.f, -2.f)
	, mLookAt(0.f, 1.f, 0.f)
	, mUp(0.f, 1.f, 0.f)
{}

CameraTask::~CameraTask()
{}

bool CameraTask::load()
{
	if (auto* script = ResourceManager::instance().load<ResourceLua>(L"data/script/cameracontroller.lua")) {
		if (!script->isValid()) {
			return false;
		}
		mScript = script;
	}
	return true;
}

void CameraTask::fixup()
{}

void CameraTask::update()
{
	auto ratio = this->getRatio();
}

void CameraTask::fixedUpdate()
{
	if (SceneManager::instance().isPaused()) {
		return;
	}

	auto const elapsed = this->getScene().getElapsedSecond();
	auto const ratio = this->getRatio();
	auto& L = mScript->getContext();

	lua_getglobal(L, "get_position");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	lua_pushnumber(L, ratio);
	if (lua_pcall(L, 2, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	lua_pushstring(L, "posx");
	lua_gettable(L, -2);
	auto posx = lua_tonumber(L, -1);
	lua_pushstring(L, "posy");
	lua_gettable(L, -3);
	auto posy = lua_tonumber(L, -1);
	lua_pushstring(L, "posz");
	lua_gettable(L, -4);
	auto posz = lua_tonumber(L, -1);
	lua_pushstring(L, "atx");
	lua_gettable(L, -5);
	auto atx = lua_tonumber(L, -1);
	lua_pushstring(L, "aty");
	lua_gettable(L, -6);
	auto aty = lua_tonumber(L, -1);
	lua_pushstring(L, "atz");
	lua_gettable(L, -7);
	auto atz = lua_tonumber(L, -1);
	lua_pop(L, 7);

	mPosition = XMFLOAT3(posx, posy, posz);
	mLookAt = XMFLOAT3(atx, aty, atz);
}

void CameraTask::draw(Renderer& renderer)
{
	auto& draw2d = renderer.getDraw2D();
	draw2d.drawText(); // TODO:

//	XMFLOAT3 pos, at, up;
//	pos.x = 2.f; pos.y = 2.8f; pos.z = -2.f;
//	at.x = 0.f; at.y = 1.f; at.z = 0.f;
//	up.x = 0.f; up.y = 1.f; up.z = 0.f;
	renderer.setMainCamera(mPosition, mLookAt, mUp);
}

void CameraTask::reset()
{}

}
