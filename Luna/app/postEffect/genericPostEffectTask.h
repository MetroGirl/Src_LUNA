//
// Generic PostEffect Task.
//

#ifndef LUNA_GENERICPOSTEFFECTTASK_TASK_H_INCLUDED
#define LUNA_GENERICPOSTEFFECTTASK_TASK_H_INCLUDED

#include "lib/task.h"
#include "app/postEffect/postEffectTask.h"

namespace luna{
	class GenericPostEffectTask : public PostEffectTask
	{
		LUNA_DECLARE_CONCRETE(GenericPostEffectTask, PostEffectTask);

	public:
		GenericPostEffectTask();

		virtual ~GenericPostEffectTask() override;

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
		struct PatchInfo
		{
			s32 registerSlot;
			bool isResource;
			union {
				class TextureResource* resourcePtr;
				s32 fbSlot;
			};
		};
		struct EffectInfo
		{
			f32 start;
			f32 end;
			string techniqueName;
			vector<PatchInfo> patchTbl;
			XMFLOAT4 metadata1;
			XMFLOAT4 metadata2;
		};
		struct EffectCB
		{
			XMFLOAT4 sceneRatio;
			XMFLOAT4 effectRatio;
			XMFLOAT4 metadata1;
			XMFLOAT4 metadata2;
		};

		ResourceLua* mResourcePtr;
		vector<EffectInfo> mEffectTbl;
		EffectCB mEffect;
		ComPtr<ID3D11Buffer> mEffectCB;
	};
}

#endif // LUNA_GENERICPOSTEFFECTTASK_TASK_H_INCLUDED
