//
// debug functionalities.
//

#ifndef LUNA_DEBUG_H_INCLUDED
#define LUNA_DEBUG_H_INCLUDED

namespace luna{
	//! @brief デバッグ
	class Debug : public Singleton<Debug>
	{
	public:
#if !LUNA_PUBLISH
		void allocateConsole();
		void freeConsole();

		void trace(const c16* format, ...);
		void traceLine(const c16* format, ...);
		void warning(const c16* format, ...);
		void warningLine(const c16* format, ...);
		void error(const c16* format, ...);
		void errorLine(const c16* format, ...);
#else
		void allocateConsole(){}
		void freeConsole(){}
#endif
	};
}

//! @brief トラップ命令を発行します
#if !LUNA_PUBLISH
#define LUNA_DEBUG_BREAK()\
	LUNA_INTRINSIC_DEBUG_BREAK()
#else
#define LUNA_DEBUG_BREAK() do{}while(0)
#endif

//! @brief 文字列をコンソールへ出力します
#if !LUNA_PUBLISH
#define LUNA_TRACE(...)\
	luna::Debug::instance().trace(__VA_ARGS__)
#else
#define LUNA_TRACE(...) do{}while(0)
#endif

//! @brief 文字列を改行コード付きでコンソールへ出力します
#if !LUNA_PUBLISH
#define LUNA_TRACELINE(...)\
	luna::Debug::instance().traceLine(__VA_ARGS__)
#else
#define LUNA_TRACELINE(...) do{}while(0)
#endif

//! @brief 文字列をコンソールへ出力します
#if !LUNA_PUBLISH
#define LUNA_WARNING(...)\
	luna::Debug::instance().warning(__VA_ARGS__)
#else
#define LUNA_WARNING(...) do{}while(0)
#endif

//! @brief 文字列を改行コード付きでコンソールへ出力します
#if !LUNA_PUBLISH
#define LUNA_WARNINGLINE(...)\
	luna::Debug::instance().warningLine(__VA_ARGS__)
#else
#define LUNA_WARNINGLINE(...) do{}while(0)
#endif

//! @brief 文字列をコンソールへ出力します
#if !LUNA_PUBLISH
#define LUNA_ERROR(...)\
	luna::Debug::instance().error(__VA_ARGS__)
#else
#define LUNA_ERROR(...) do{}while(0)
#endif

//! @brief 文字列を改行コード付きでコンソールへ出力します
#if !LUNA_PUBLISH
#define LUNA_ERRORLINE(...)\
	luna::Debug::instance().errorLine(__VA_ARGS__)
#else
#define LUNA_ERRORLINE(...) do{}while(0)
#endif

#endif // LUNA_DEBUG_H_INCLUDED
