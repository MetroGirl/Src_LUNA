#include "stdafx.h"
#include "entryPoint.h"
#include "instance.h"

namespace luna{
	s32 entryPoint(s32 argc, char** argv)
	{
		LUNA_TRACELINE(L"Luna is now getting the rhythm...");

		// デバッグウィンドウの確保
		luna::Debug::instance().allocateConsole();

		// 型情報ツリーのデバッグ
		TypeInfo::dump();

		// コアシステムの初期化
		if (!SettingsManager::instance().initialize()){
			ExitProcess(0);
		}
		ResourceManager::instance().initialize();
		ScriptManager::instance().initialize();

		// アプリケーションインスタンスの作成
		vector<unique_ptr<Instance>> instanceTbl;
		auto* ti = Instance::TypeInfo.getChild();
		while (ti){
			if (ti->isAbstract()){
				continue;
			}

			instanceTbl.emplace_back(unique_ptr<Instance>(ti->createInstance<Instance>()));
			ti = ti->getNext();
		}
		LUNA_ASSERT(!instanceTbl.empty(), L"implement class inherits Instance class.");

		// 初期化
		for (auto& instancePtr : instanceTbl){
			instancePtr->initialize();
		}

		// ジョブ実行
#if 0//LUNA_WINDOWS
		MSG msg;
		bool running = true;
		while (running){
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					running = false;
				} else {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}else{
			}
		}
#endif

		bool running = true;
		while(running){
			for (auto& instancePtr : instanceTbl){
				if (!instancePtr->isRunnable()){
					continue;
				} 

				if(!instancePtr->run()){
					instancePtr->setRunnable(false);
				}

				// シングルトンの実行
				ResourceManager::instance().postUpdate();
			}

			if (none_of(instanceTbl.begin(), instanceTbl.end(), [&](unique_ptr<Instance>& instancePtr){ return instancePtr->isRunnable(); })){
				running = false;
			}
		}

#if !LUNA_PUBLISH
		// 破棄
		for (auto& instancePtr : instanceTbl){
			instancePtr->finalize();
		}

		// コアシステムの破棄
		ScriptManager::instance().finalize();
		ResourceManager::instance().finalize();
#else
		ExitProcess(0);
#endif

		// デバッグウィンドウの破棄
		luna::Debug::instance().freeConsole();

		LUNA_TRACELINE(L"Luna halted.");
		return 0;
	}
}
