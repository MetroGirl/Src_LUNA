#include "stdafx.h"
#include "fluidGridTask.h"
#include "renderer.h"
#include "renderPass/fluidPass.h"
#include "renderPass/draw2dPass.h"
#include "stringHelper.h"

namespace luna {

LUNA_IMPLEMENT_CONCRETE(luna::FluidGridTask);

FluidGridTask::FluidGridTask()
	: mScript(nullptr)
	, mGrid(createFluidGrid(XMUINT3(64,64,64), XMFLOAT3(3.f, 3.f, 3.f)))
	, mReset(false)
	, mDispatchFlag(true)
{}

FluidGridTask::~FluidGridTask()
{
	if (mScript) mScript->release();
}

bool FluidGridTask::load()
{
	if (auto* script = ResourceManager::instance().load<ResourceLua>(L"data/script/fluidcontroller.lua")) {
		if (!script->isValid()) {
			return false;
		}
		mScript = script;
	}
	auto& L = mScript->getContext();

	lua_getglobal(L, "get_initial_position");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	if (lua_pcall(L, 0, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return false;
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
	lua_pop(L, 4);

	mGrid->mGridPosition = mGrid->mPreviousGridPosition = XMFLOAT3(posx*0.1f, posy*0.1f, posz*0.1f);

	return true;
}

void FluidGridTask::fixup()
{
}

void FluidGridTask::update()
{
}

void FluidGridTask::fixedUpdate()
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
	lua_pop(L, 4);

	mGrid->mPreviousGridPosition = mGrid->mGridPosition;
	mGrid->mGridPosition = XMFLOAT3(posx, posy, posz);

	lua_getglobal(L, "get_pressure_sources");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	if (lua_pcall(L, 1, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	if (!lua_isnil(L, -1)) {
		for (int i = 1;; ++i) {
			lua_pushinteger(L, i);
			lua_gettable(L, -2);
			if (lua_type(L, -1) != LUA_TTABLE) break;

			lua_pushstring(L, "posx");
			lua_gettable(L, -2);
			auto posx = lua_tonumber(L, -1);
			lua_pushstring(L, "posy");
			lua_gettable(L, -3);
			auto posy = lua_tonumber(L, -1);
			lua_pushstring(L, "posz");
			lua_gettable(L, -4);
			auto posz = lua_tonumber(L, -1);
			lua_pushstring(L, "radius");
			lua_gettable(L, -5);
			auto radius = lua_tonumber(L, -1);
			lua_pushstring(L, "pressure");
			lua_gettable(L, -6);
			auto pressure = lua_tonumber(L, -1);
			lua_pop(L, 6);
//			lua_pop(L, 1);

			PressureSource ps;
			ps.mPosition = XMFLOAT3(posx, posy, posz);
			ps.mRadius = radius;
			ps.mPressure = pressure;
			mPressureSources.push_back(ps);
		}
	}

	lua_getglobal(L, "do_reset");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	if (lua_pcall(L, 1, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	auto const isReset = lua_toboolean(L, -1)? true: false;
	mReset = isReset;
	mDispatchFlag = true;
}

void FluidGridTask::draw(Renderer& renderer)
{
	auto& fluid = renderer.getFluid();
	if (!SceneManager::instance().isPaused()) {
		// test
		if (mDispatchFlag) {
			fluid.simulateGrid(mGrid, mReset);
			for(auto const& ps : mPressureSources) {
				fluid.addPressureSource(ps);
			}
			mDispatchFlag = false;
		}
		mPressureSources.clear();
	}

	auto& fluidRender = renderer.getFluidRender();
	fluidRender.renderGrid(mGrid);

	auto& draw2d = renderer.getDraw2D();
	draw2d.drawText(); // TODO:
}

void FluidGridTask::reset()
{}

}
