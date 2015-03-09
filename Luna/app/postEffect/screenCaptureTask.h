//
// Screen capture Task.
//

#ifndef LUNA_SCREENCAPTURETASK_H_INCLUDED
#define LUNA_SCREENCAPTURETASK_H_INCLUDED

#include "lib/task.h"

namespace luna{
	//! @brief オーバーレイフェード用
	class ScreenCaptureTask : public Task
	{
		LUNA_DECLARE_CONCRETE(ScreenCaptureTask, Task);

	public:
		ScreenCaptureTask();

		virtual ~ScreenCaptureTask() override;

		//! @brief シーンに必要なデータをロード
		//! @returns ロードが完了したか否か
		virtual bool load() override;

		//! @brief 準備
		//! このメソッドでタスクはセットアップを完了します
		//! load()完了後、最初のupdate()の前に一度だけ呼び出されます
		virtual void fixup() override;

		//! @brief 更新
		virtual void update() override;

		//! @brief 固定フレーム更新
		//! @note 呼び出される頻度はScene::getFixedUpdateInterval()によって取得することができます
		virtual void fixedUpdate() override;

		//! @brief 描画
		virtual void draw(Renderer& renderer) override;

		//! @brief 描画コールバック
		virtual void onRender(const luna::TypeInfo& type, RenderPassContext& rpc, Renderer& renderer, u32& arg) override;

		//! @brief リセット
		//! 実行状態をリセットし、再実行に備える
		//! @note 古典的デモオブジェクトの場合はratioによって状態が決定するので何もする必要はない
		virtual void reset() override;

	private:
		void updateEffectTbl();

	private:
		struct EffectInfo
		{
			f32 timing;
			u32 slot;
			bool consumed;
			std::string technique;
		};

		ResourceLua* mResourcePtr;
		vector<EffectInfo> mEffectTbl;
	};
}

#endif // LUNA_SCREENCAPTURETASK_H_INCLUDED
