//
// Coding style.
//

// インクルードガードは#ifndef - #define - #endif方式
#ifndef LUNA_CONDINGSTYLE_H_INCLUDED
#define LUNA_CONDINGSTYLE_H_INCLUDED

// マクロ以外の要素は 'luna' 名前空間に配置する
namespace luna{
	// 標準ライブラリの機能を使う場合、一度 luna 名前空間へusingする
	using std::wstring;

	// 組み込み型はサイズがわかるようにtypedefしたものを用いる
	s32 getHoge(); // 正解
	int getFoo(); // 誤り

	// クラス名はUpperCamelスタイル
	class HogeFooBar
	{
		// メソッド名はlowerCamelスタイル
		void doSomething();

		// メンバ変数名は m で始める
		int mValue;
	};
}

#endif // LUNA_CONDINGSTYLE_H_INCLUDED