#include "stdafx.h"
#include "particleEmitterTask.h"
#include "renderer.h"
#include "renderPass/particlePass.h"
#include "renderPass/draw2dPass.h"
#include "stringHelper.h"

namespace luna {

LUNA_IMPLEMENT_CONCRETE(luna::ParticleEmitterTask);

ParticleEmitterTask::ParticleEmitterTask()
	: mScript(nullptr)
	, mDispatchFlag(true)
{}

ParticleEmitterTask::~ParticleEmitterTask()
{
	if (mScript) mScript->release();
}

bool ParticleEmitterTask::load()
{
	if (auto* script = ResourceManager::instance().load<ResourceLua>(L"data/script/emittercontroller.lua")) {
		if (!script->isValid()) {
			return false;
		}
		mScript = script;
	}
	
	auto& L = mScript->getContext();
	lua_getglobal(L, "add_triggers");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	if (lua_pcall(L, 0, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return false;
	}
	if (!lua_isnil(L, -1)) {
		for (int i = 1;; ++i) {
			lua_pushinteger(L, i);
			lua_gettable(L, -2);
			if (lua_isnil(L, -1)) break;
			auto const trigger = lua_tonumber(L, -1);
			this->addTrigger(trigger);
			lua_pop(L, 1);
		}
	}
	lua_pop(L, 1);
	return true;
}

void ParticleEmitterTask::fixup()
{}

void ParticleEmitterTask::triggerUpdate(f32 elapsed)
{
	auto const sceneElapsed = this->getScene().getElapsedSecond();
	auto const ratio = this->getRatio();
	auto& L = mScript->getContext();

	lua_getglobal(L, "get_trigger_emitter");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	if (lua_pcall(L, 1, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	if (lua_isnil(L, -1)) return;
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
	lua_pushstring(L, "count");
	lua_gettable(L, -6);
	auto count = lua_tonumber(L, -1);
	lua_pushstring(L, "lifetime");
	lua_gettable(L, -7);
	auto lifetime = lua_isnil(L, -1) ? 240.f : lua_tonumber(L, -1);
	lua_pushstring(L, "index");
	lua_gettable(L, -8);
	auto index = lua_isnil(L, -1) ? 0 : lua_tonumber(L, -1);
	lua_pop(L, 8);
 
	EmitterAttribute emitter;
	emitter.mEmitterRadius = radius;
	emitter.mEmitterPosition = XMFLOAT3(posx, posy, posz);
	emitter.mEmitCount = count;
	emitter.mEmitLifeTime = lifetime;
	emitter.mUniversalIndex = index;
	mEmitters.push_back(emitter);
}

void ParticleEmitterTask::update()
{
}

void ParticleEmitterTask::fixedUpdate()
{
	if (SceneManager::instance().isPaused()) {
		return;
	}

	auto const elapsed = this->getScene().getElapsedSecond();
	auto const ratio = this->getRatio();
	auto& L = mScript->getContext();

	lua_getglobal(L, "get_emitter");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	lua_pushnumber(L, ratio);
	if (lua_pcall(L, 2, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	if (!lua_isnil(L, -1)) {
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		if (lua_istable(L, -1)) {
			lua_pop(L, 1);
			for (int i = 1;; ++i) {
				lua_pushinteger(L, i);
				lua_gettable(L, -2);
				if (lua_isnil(L, -1)) break;

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
				lua_pushstring(L, "count");
				lua_gettable(L, -6);
				auto count = lua_tonumber(L, -1);
				lua_pushstring(L, "lifetime");
				lua_gettable(L, -7);
				auto lifetime = lua_isnil(L, -1) ? 240.f : lua_tonumber(L, -1);
				lua_pushstring(L, "index");
				lua_gettable(L, -8);
				auto index = lua_isnil(L, -1) ? 0: lua_tonumber(L, -1);
				lua_pop(L, 8);

		EmitterAttribute emitter;
		emitter.mEmitterRadius = radius;
		emitter.mEmitterPosition = XMFLOAT3(posx, posy, posz);
		emitter.mEmitCount = count;
		emitter.mEmitLifeTime = lifetime;
		emitter.mUniversalIndex = index;
		mEmitters.push_back(emitter);
		mDispatchFlag = true;
			}
		}
		else {
			lua_pop(L, 1);
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
		lua_pushstring(L, "count");
		lua_gettable(L, -6);
		auto count = lua_tonumber(L, -1);
		lua_pushstring(L, "lifetime");
		lua_gettable(L, -7);
		auto lifetime = lua_isnil(L, -1) ? 240.f : lua_tonumber(L, -1);
		lua_pushstring(L, "index");
		lua_gettable(L, -8);
		auto index = lua_isnil(L, -1) ? 0 : lua_tonumber(L, -1);
		lua_pop(L, 8);

		EmitterAttribute emitter;
		emitter.mEmitterRadius = radius;
		emitter.mEmitterPosition = XMFLOAT3(posx, posy, posz);
		emitter.mEmitCount = count;
		emitter.mEmitLifeTime = lifetime;
		emitter.mUniversalIndex = index;
		mEmitters.push_back(emitter);
		mDispatchFlag = true;
		}
	}

	lua_getglobal(L, "get_shader");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	if (lua_pcall(L, 1, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	auto shaderName = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_getglobal(L, "get_render_shader");
	LUNA_ASSERT(lua_type(L, -1) == LUA_TFUNCTION, L"");
	lua_pushnumber(L, elapsed);
	if (lua_pcall(L, 1, 1, 0)) {
		LUNA_ERRORLINE(L"LUA: %ls", ascii_to_wide(lua_tostring(L, -1)).data());
		return;
	}
	auto renderShaderName = lua_tostring(L, -1);
	lua_pop(L, 1);

	mShaderName = shaderName;
	mRenderShaderName = renderShaderName;
}

void ParticleEmitterTask::draw(Renderer& renderer)
{
	if (SceneManager::instance().isPaused()) {
		return;
	}

	auto& sim = renderer.getParticleSimulation();

	sim.setShader(mShaderName);
	if (mDispatchFlag) {
	for (auto const& attribute : mEmitters) {
		sim.addEmitterController([attribute](ParticleEmitterSettings& emitter) {
			emitter.mEmitter.mPosition = attribute.mEmitterPosition;
			emitter.mEmitter.mRadius = attribute.mEmitterRadius;
			emitter.mEmitter.mColor = XMFLOAT4(0.02f, 0.02f, 0.02f, 0.8f);
			emitter.mEmitter.mEmitCount = attribute.mEmitCount;
			emitter.mEmitter.mEmitLifeTime = attribute.mEmitLifeTime;
			emitter.mEmitter.mUniversalIndex = attribute.mUniversalIndex;
			emitter.mShaderName = "particle_emit_in_sphere";
		});
	}
	mDispatchFlag = false;
	}
	mEmitters.clear();

	auto& pr = renderer.getParticleRender();
	pr.setShader(mRenderShaderName);

	auto& draw2d = renderer.getDraw2D();
	draw2d.drawText(); // TODO:
}

void ParticleEmitterTask::reset()
{}

}
