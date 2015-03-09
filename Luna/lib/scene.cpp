#include "stdafx.h"
#include "scene.h"
#include "task.h"
#include "app/renderer.h"// ugly...

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::Scene);

	Scene::Scene()
	{
		LUNA_ASSERT(0, "do not create instance via new. use SceneManager::pushScene() instead.");
	}

	Scene::Scene(const c16* name, f32 start, f32 end, f32 fixedInterval)
		: mFirstUpdate(true)
		, mActive(false)
		, mStartSecond(start)
		, mEndSecond(end)
		, mFixedUpdateIntervalMilliseconds(fixedInterval)
		, mName(name)
		, mCurrentFrameRatio(0)
		, mElapsedSecond(0)
		, mLayer(SceneLayer_0)
	{
	}

	bool Scene::load()
	{
		for(auto& taskPtr : mTaskTbl){
			if (!taskPtr->load()){
				return false;
			}
		}
		return true;
	}

	void Scene::fixup()
	{
		for (auto& taskPtr : mTaskTbl){
			taskPtr->fixup();
		}
	}

	bool Scene::run(f32 demoSecond)
	{
		setCurrentFrameRatio(calcurateRatio(demoSecond));
		setElapsedSecond(demoSecond - mStartSecond);

		if (mFirstUpdate){
			LUNA_TRACELINE(L"[Scene]%sの再生開始(%.3f ～ %.3f)", getName(), mStartSecond, mEndSecond);
		}
		if (mTaskTbl.empty()){
			LUNA_WARNINGLINE(L"[Scene]%s 実行するタスクがありません", getName());
		}

		// update 
		for (auto& taskPtr : mTaskTbl){
			taskPtr->update();
		}

		// fixed-update
		if (mFirstUpdate || mWatch.getElapsedMilliseconds() >= getFixedUpdateInterval())
		{
			for (auto& taskPtr : mTaskTbl){
				taskPtr->fixedUpdate();
			}
			mWatch.Reset();
		}

		// trigger-update
		for (auto& taskPtr : mTaskTbl){
			taskPtr->fireTrigger(demoSecond);
		}

		// draw
		for (auto& taskPtr : mTaskTbl){
			taskPtr->draw(Renderer::instance());
		}

		mFirstUpdate = false;

		return true;
	}

	void Scene::reset()
	{
		for (auto& taskPtr : mTaskTbl){
			taskPtr->doReset();
		}
		mFirstUpdate = true;
	}

	Task* Scene::createTask(const luna::TypeInfo& ti, const c16* name)
	{
		auto* taskPtr = static_cast<Task*>(ti.createInstance());
		taskPtr->setName(name);
		taskPtr->setScene(this);
		mTaskTbl.push_back(unique_ptr<Task>(taskPtr));
		return taskPtr;
	}

	void Scene::onControlJump(f32 second)
	{
		for (auto& task : mTaskTbl){
			task->onControlJump(second);
		}
	}

	bool Scene::hasTaskOfType(const luna::TypeInfo& ti) const
	{
		for (auto& task : mTaskTbl){
			if (task->getTypeInfo().isA(ti)){
				return true;
			}
		}
		return false;
	}
}
