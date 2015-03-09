//
// game instance.
//

#ifndef LUNA_INSTANCE_H_INCLUDED
#define LUNA_INSTANCE_H_INCLUDED

#include "object.h"

namespace luna{
	//! @brief アプリケーションインスタンス基底
	class Instance : public Object
	{
		LUNA_DECLARE_ABSTRACT(Instance, Object);

	public:
		Instance()
			: mRunnable(true)
		{
		}

		//! @brief インスタンスの初期化
		virtual void initialize() = 0;

		//! @brief インスタンスの実行
		virtual bool run() = 0;

		//! @brief インスタンスの破棄
		virtual void finalize() = 0;

		bool isRunnable() const
		{
			return mRunnable;
		}

		void setRunnable(bool runnable)
		{
			mRunnable = runnable;
		}

	private:
		bool mRunnable;
	};
}

#endif // LUNA_INSTANCE_H_INCLUDED

