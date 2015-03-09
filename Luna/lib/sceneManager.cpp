#include "stdafx.h"
#include "sceneManager.h"
#include "soundManager.h"
#include "app/renderer.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::SceneManager);
	LUNA_IMPLEMENT_SINGLETON(luna::SceneManager);

	SceneManager::SceneManager()
		: mElapsedSecond(0)
		, mReady(false)
		, mFirstUpdate(true)
	{
	}

	SceneManager::~SceneManager()
	{
	}

	bool SceneManager::load()
	{
		for (auto& scenePtr : mSceneTbl){
			if (!scenePtr->load()){
				return false;
			}
		}
		return true;
	}

	void SceneManager::fixup()
	{
		for (auto& scenePtr : mSceneTbl){
			scenePtr->fixup();
		}
		for (auto& scenePtr : mSceneTbl){
			scenePtr->reset();
		}
		mReady = true;
	}

	bool SceneManager::run()
	{
		if (mFirstUpdate){
			SoundManager::instance().play(0);
			mFirstUpdate = false;
		}

		// 音の再生時間にクランプ
		const f32 estimatedSecond = SoundManager::instance().getCurrentTime() / 1000.f;
		const f32 soundDuration = SoundManager::instance().getDuration() / 1000.0f;// SoundManager::instance().getCurrentTime() / 1000.0f;
		if (estimatedSecond >= soundDuration)
		{
			bool isLoopDemo = SettingsManager::instance().isLoopDemo();
#if !LUNA_PUBLISH
			isLoopDemo = true;
#endif
			if (isLoopDemo)
			{
				LUNA_TRACELINE(L"[SceneManager]Loop.");
				SoundManager::instance().play(0);
			}
			else
			{
				ExitProcess(0);
			}
		}

		// 再生時間の更新
		setElapsedSecond(SoundManager::instance().getCurrentTime()/1000.f);
		setDuration((f32)SoundManager::instance().getDuration()/1000.f);

		// シーンの更新
		const f32 elapsed = getElapsedSecond();

		u32 layerIndex = 0;
		for (auto& scenePtr : mSceneTbl){
			if (!scenePtr->isRunnable(elapsed)){
				scenePtr->setActive(false);
				continue;
			}

			scenePtr->setLayer((SceneLayer)layerIndex++);
			scenePtr->setActive(true);
			if (!scenePtr->run(elapsed)){
				return false;
			}
		}
		if (none_of(mSceneTbl.begin(), mSceneTbl.end(), [&](unique_ptr<Scene>& scenePtr){ return scenePtr->isRunnable(elapsed); })){
			LUNA_ERRORLINE(L"再生対象のSceneがひとつも存在しない %.3f秒地点", elapsed);
		}

		return true;
	}

	Scene* SceneManager::pushScene(const c16* name, f32 start, f32 end, f32 fixedInterval/* = 1000.0f/30.0f */)
	{
		auto* scenePtr = new Scene(name, start, end, fixedInterval);
		mSceneTbl.push_back(unique_ptr<Scene>(scenePtr));
		return scenePtr;
	}

	void SceneManager::onControlJump(f32 second)
	{
		const f32 origin = getElapsedSecond();

		const bool wasPlaying = SoundManager::instance().isPlaying();
		if (!wasPlaying){
			SoundManager::instance().setVolume(0.f);
		}
		SoundManager::instance().play((u32)(second*1000.f));
		if (!wasPlaying){
			SoundManager::instance().stop();
			SoundManager::instance().setVolume(1.f);
		}

		// 時間を上書き
		setElapsedSecond(SoundManager::instance().getCurrentTime() / 1000.f);
		setDuration((f32)SoundManager::instance().getDuration() / 1000.f);

		// シーンの経過時間と関連するステートのリセット
		for (auto& scene : mSceneTbl){
			scene->onControlJump(second);
		}

		LUNA_TRACELINE(L"[SceneManager]Jumped from %.3f to %.3f", origin, second);
	}

	array<f32, SceneLayer_Num> SceneManager::getSceneElapsedSecond()
	{
		size_t writePt = 0;
		array<f32, SceneLayer_Num> retTbl = {};
		for (auto& scenePtr : mSceneTbl){
			if (scenePtr->isActive()){
				retTbl[writePt++] = scenePtr->getElapsedSecond();
				if (writePt >= retTbl.size()){
					break;
				}
			}
		}
		return retTbl;
	}

	array<f32, SceneLayer_Num> SceneManager::getSceneDuration()
	{
		size_t writePt = 0;
		array<f32, SceneLayer_Num> retTbl = {};
		for (auto& scenePtr : mSceneTbl){
			if (scenePtr->isActive()){
				retTbl[writePt++] = scenePtr->getDuration();
				if (writePt >= retTbl.size()){
					break;
				}
			}
		}
		return retTbl;
	}

	array<f32, SceneLayer_Num> SceneManager::getSceneRatio()
	{
		size_t writePt = 0;
		array<f32, SceneLayer_Num> retTbl = {};
		for (auto& scenePtr : mSceneTbl){
			if (scenePtr->isActive()){
				retTbl[writePt++] = scenePtr->getCurrentFrameRatio();
				if (writePt >= retTbl.size()){
					break;
				}
			}
		}
		return retTbl;
	}

#if !LUNA_PUBLISH
	void SceneManager::play()
	{
		if (!SoundManager::instance().isPlaying()){
			SoundManager::instance().play();
		}
	}

	void SceneManager::pause()
	{
		if (SoundManager::instance().isPlaying()){
			SoundManager::instance().stop();
		}
		LUNA_TRACE(L"[second] %f\n", this->getSceneElapsedSecond()[1]);
		LUNA_TRACE(L"[ratio] %f\n", this->getSceneRatio()[0]);
	}

	bool SceneManager::isPaused()
	{
		return !SoundManager::instance().isPlaying();
	}

	void SceneManager::nextScene()
	{

	}

	void SceneManager::prevScene()
	{

	}

	void SceneManager::fastForward()
	{

	}

	void SceneManager::rewind()
	{

	}
#endif
}
