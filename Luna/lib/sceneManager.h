//
// Scene Manager.
//

#ifndef LUNA_SCENE_MANAGER_H_INCLUDED
#define LUNA_SCENE_MANAGER_H_INCLUDED

#include "lib/object.h"

namespace luna{
	class Scene;

	enum SceneLayer
	{
		SceneLayer_0,
		SceneLayer_1,
		SceneLayer_2,
		SceneLayer_3,

		SceneLayer_Num
	};

	//! @brief シーン管理
	//! 
	//! Sceneクラスの管理と実行を行います
	//! デモ全体の時間管理を担当します
	//! 
	//! 通常、Demoクラスによって駆動されます
	class SceneManager : public Object, public Singleton<SceneManager>
	{
		LUNA_DECLARE_CONCRETE(SceneManager, Object);

	public:
		SceneManager();
		~SceneManager();

		//! @brief 全シーンのロード
		bool load();

		//! @brief 全シーンの準備
		void fixup();

		//! @brief デモ実行
		bool run();

		bool isReady() const
		{
			return mReady;
		}

		void setReady(bool v)
		{
			mReady = v;
		}

		Scene* pushScene(const c16* name, f32 start, f32 end, f32 fixedInterval = 1000.0f / 30.0f);

		f32 getElapsedSecond() const
		{
			return mElapsedSecond;
		}

		void setElapsedSecond(f32 v)
		{
			mElapsedSecond = v;
		}

		f32 getDuration() const
		{
			return mDuration;
		}

		void setDuration(f32 v)
		{
			mDuration = v;
		}

		const vector<unique_ptr<Scene>>& getSceneTbl() const
		{
			return mSceneTbl;
		}

		//! @brief コントロールによる再生時間ジャンプ
		void onControlJump(f32 second);


		array<f32, SceneLayer_Num> getSceneElapsedSecond();
		array<f32, SceneLayer_Num> getSceneDuration();
		array<f32, SceneLayer_Num> getSceneRatio();

#if !LUNA_PUBLISH
		void play();
		void pause();
		bool isPaused();
		void nextScene();
		void prevScene();
		void fastForward();
		void rewind();
#else
		bool isPaused() const
		{
			return false;
		}
#endif

	private:
		bool mReady;
		bool mFirstUpdate;
		f32 mElapsedSecond;
		f32 mDuration;
		vector<unique_ptr<Scene>> mSceneTbl;
	};
}

#endif // LUNA_SCENE_MANAGER_H_INCLUDED
