//
// Scene.
//

#ifndef LUNA_SCENE_H_INCLUDED
#define LUNA_SCENE_H_INCLUDED

#include "lib/object.h"

namespace luna{
	class Task;

	//! @brief シーン
	//! 
	//! デモ内のひとつのシーンを表現します
	//! 開始時間と終了時間を持ち、その間はrun()が呼び出され続けます
	//! 
	//! 通常、SceneManagerクラスによって駆動されます
	class Scene : public Object
	{
		LUNA_DECLARE_CONCRETE(Scene, Object);

	public:
		Scene(const c16* name, f32 start, f32 end, f32 fixedInterval);

		//! @brief シーンに必要なデータをロード
		//! @returns ロードが完了したか否か
		bool load();

		//! @brief 準備
		//! このメソッドでセットアップを完了します
		//! load()完了後に一度だけ呼び出されます
		void fixup();

		//! @brief 実行
		//! @returns 実行を続けても良いか否か
		bool run(f32 ratio);

		//! @brief 実行状態のリセット
		void reset();

		bool isActive() const
		{
			return mActive;
		}

		void setActive(bool v)
		{
			if (mActive != v && v == false){
				// 無効になった瞬間は次回の周回に備えてステートをリセットしておく
				reset();
				LUNA_TRACELINE(L"[Scene]%lsのリセット", mName.c_str());
			}
			mActive = v;
		}

		bool isRunnable(f32 second)
		{
			return mStartSecond <= second && second < mEndSecond;
		}

		f32 getStartSecond() const
		{
			return mStartSecond;
		}

		f32 getEndSecond() const
		{
			return mEndSecond;
		}

		f32 calcurateRatio(f32 second)
		{
			return (second - mStartSecond) / getDuration();
		}

		f32 getCurrentFrameRatio() const
		{
			return mCurrentFrameRatio;
		}

		void setCurrentFrameRatio(f32 ratio)
		{
			mCurrentFrameRatio = ratio;
		}

		const c16* getName() const
		{
			return mName.c_str();
		}

		f32 getFixedUpdateInterval() const
		{
			return mFixedUpdateIntervalMilliseconds;
		}

		void setFixedUpdateInterval(f32 v)
		{
			mFixedUpdateIntervalMilliseconds = v;
		}

		template<typename T>
		T* createTask(const c16* name)
		{
			return static_cast<T*>(createTask(T::TypeInfo, name));
		}

		Task* createTask(const luna::TypeInfo& ti, const c16* name);

		f32 getDuration() const
		{
			return mEndSecond - mStartSecond;
		}

		f32 getElapsedSecond() const
		{
			return mElapsedSecond;
		}

		void setElapsedSecond(f32 v)
		{
			mElapsedSecond = v;
		}

		SceneLayer getLayer() const
		{
			return mLayer;
		}

		void setLayer(SceneLayer v)
		{
			mLayer = v;
		}

		//! @brief 編集時ジャンプ
		void onControlJump(f32 second);

		bool hasTaskOfType(const luna::TypeInfo& ti) const;

	private:
		Scene();

	private:
		bool mActive;
		bool mFirstUpdate;

		f32 mStartSecond;
		f32 mEndSecond;
		f32 mElapsedSecond;// domestic second.
		f32 mCurrentFrameRatio;
		SceneLayer mLayer;

		f32 mFixedUpdateIntervalMilliseconds;//!< msec.
		StopWatch mWatch;

		wstring mName;
		vector<unique_ptr<Task>> mTaskTbl;
	};
}

#endif // LUNA_SCENE_H_INCLUDED
