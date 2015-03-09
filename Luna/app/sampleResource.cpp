#include "stdafx.h"
#include "sampleResource.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::SampleResource);

	SampleResource::SampleResource()
	{
		// 状態の初期化のみを行ってください
	}

	SampleResource::~SampleResource()
	{
	}

	bool SampleResource::load(Stream& fs)
	{
		// 実データのロード
		unique_ptr<c8[]> rawStr(new c8[fs.getSize() + 1]);
		if (fs.read(rawStr.get(), fs.getSize()) != fs.getSize()){
			return false;
		}
		rawStr.get()[fs.getSize()] = '\0';

		mData = rawStr.get();

		return true;
	}

	bool SampleResource::isLoadable(Stream& fs)
	{
		// このメソッドでは、fsを用いてロード可能か否かを返します
		// 単純な拡張子による比較でも構いませんし、
		// ヘッダを読み取って判定しても構いません。

		// サンプルなので拡張子だけで判断
		return fs.getExtension() == L"txt";
	}

	void SampleResource::freeResource()
	{
		// 読み込んだデータを解放します
		mData.clear();
	}
}

