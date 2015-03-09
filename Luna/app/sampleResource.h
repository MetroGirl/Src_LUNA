//
// Sample Resource.
//

#ifndef LUNA_SAMPLE_RESOURCE_H_INCLUDED
#define LUNA_SAMPLE_RESOURCE_H_INCLUDED

#include "lib/task.h"

namespace luna{
	class SampleResource : public ResourceObject
	{
		LUNA_DECLARE_CONCRETE(SampleResource, ResourceObject);

	public:
		//! @brief コンストラクタ
		//! コンストラクタでは状態の初期化のみを行います
		SampleResource();

		//! @brief デストラクタ
		virtual ~SampleResource() override;

		//! @brief ロード
		//! @returns ロードが成功したか否か
		bool load(Stream&);

		//! @brief ロード可能か否か
		//! 渡されてきたストリームを解釈可能であるか否かを返します
		//! trueを返した場合は、続けてload()が呼び出されます
		bool isLoadable(Stream&) override;

		//! @brief クリア
		//! 内部で確保しているバッファの完全な解放を行い、
		//! 新たなデータの読み込みに備えます
		void freeResource() override;

	public:
		const c8* getDataStr() const
		{
			return mData.c_str();
		}

	private:
		string mData;
	};
}

#endif // LUNA_SAMPLE_RESOURCE_H_INCLUDED
