//
// Task.
//

#ifndef LUNA_TASK_H_INCLUDED
#define LUNA_TASK_H_INCLUDED

#include "lib/object.h"

namespace luna{
	class Scene;
	class Renderer;
	class RenderPass;
	class RenderPassContext;

	//! @brief タスク
	//! 
	//! シーン中のオブジェクトを表現します
	//! 状態を持つことができ、更新と描画が明確に分離されます
	//! 
	//! かならずどこかのシーンにひも付きますが、
	//! 複数のシーンに紐づくことはありません。
	//! 
	//! ユーザーは、このクラスを継承してTaskクラスを作成します
	//! TaskクラスはSceneに管理されるため、Scene::createTask<T>()でインスタンスを生成します
	//! 
	//! 通常、Sceneクラスによって駆動されます
	class Task : public Object
	{
		LUNA_DECLARE_ABSTRACT(Task, Object);

		struct TriggerInfo
		{
			TriggerInfo(f32 second_)
				: second(second_), fired(false)
			{}

			f32 second;
			bool fired;
		};
		
	public:
		Task();

		virtual ~Task() = 0;

		//! @brief シーンに必要なデータをロード
		//! @returns ロードが完了したか否か
		virtual bool load() = 0;

		//! @brief 準備
		//! このメソッドでタスクはセットアップを完了します
		//! load()完了後、最初のupdate()の前に一度だけ呼び出されます
		virtual void fixup() = 0;

		//! @brief 更新
		virtual void update() = 0;

		//! @brief 固定フレーム更新
		//! @note 呼び出される頻度はScene::getFixedUpdateInterval()によって取得することができます
		virtual void fixedUpdate() = 0;

		//! @brief トリガ更新
		//! あらかじめ設定されているトリガタイミングで呼び出されます
		//! @param second: トリガ時間
		virtual void triggerUpdate(f32 second){}

		//! @brief 描画
		virtual void draw(Renderer& renderer) = 0;

		//! @brief 描画コールバック
		virtual void onRender(const luna::TypeInfo& type, RenderPassContext& rpc, Renderer& renderer, u32& arg){}

		//! @brief リセット
		//! 実行状態をリセットし、再実行に備える
		//! @note 古典的デモオブジェクトの場合はratioによって状態が決定するので何もする必要はない
		//! @attention このメソッドを直接呼び出さないでください。Task::doReset()経由で呼び出すことが必要です。
		virtual void reset() = 0;

		//! @brief 編集時のジャンプ
		//! 時間に関連したステートを適切にリセットします
		//! @param second : ジャンプ後のシーン全体の経過時間
		//! @note Publishでは呼び出されない
		virtual void onControlJump(f32 second)
		{
			for (auto& trigger : mTriggerTbl){
				if (second < trigger.second){
					if (trigger.fired){
						LUNA_TRACELINE(L"[Task]Reset trigger %.3f", trigger.second);
						trigger.fired = false;
					}
				}
			}
		}

		//! @brief Taskコアのリセット
		void doReset()
		{
			for (auto& trigger : mTriggerTbl){
				if (trigger.fired){
					LUNA_TRACELINE(L"[Task]Reset trigger %.3f", trigger.second);
					trigger.fired = false;
				}
			}

			reset();
		}

		//! @brief シーンの進行度合いを取得
		f32 getRatio() const
		{
			return getScene().getCurrentFrameRatio();
		}

		//! @brief 実行可能か否か
		//! Sceneが実行中の場合のみtrueを返す
		bool isRunnable() const
		{
			return getScene().isActive();
		}

		const c16* getName() const
		{
			return mName.c_str();
		}

		void setName(const c16* v)
		{
			mName = v;
		}

		const Scene& getScene() const
		{
			return *mScenePtr;
		}

		void setScene(Scene* scenePtr)
		{
			mScenePtr = scenePtr;
		}

		void addTrigger(f32 second)
		{
			mTriggerTbl.push_back(TriggerInfo(second));
		}

		bool hasTrigger(f32 second)
		{
			return any_of(mTriggerTbl.begin(), mTriggerTbl.end(), [&](const TriggerInfo& a){ return !a.fired && second > a.second; });
		}

		void fireTrigger(f32 second)
		{
			for (auto& trigger : mTriggerTbl){
				if (trigger.fired || second < trigger.second){
					continue;
				}
				LUNA_TRACELINE(L"[Task]Invoke trigger %.3f at %.3f(diff: %.3f)", trigger.second, second, second - trigger.second);
				triggerUpdate(trigger.second);
				trigger.fired = true;
			}
		}

		const vector<TriggerInfo>& getTriggerTbl() const
		{
			return mTriggerTbl;
		}

	private:
		wstring mName;
		Scene* mScenePtr;
		RenderPass* mRenderPassPtr;
		vector<TriggerInfo> mTriggerTbl;
	};
}

#endif // LUNA_TASK_H_INCLUDED
