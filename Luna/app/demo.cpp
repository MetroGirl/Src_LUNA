#include "stdafx.h"
#include "demo.h"
#include "lib/window.h"

#include "sampleTask.h"
#include "sampleResource.h"
#include "cameraTask.h"
#include "particleEmitterTask.h"
#include "shaderResource.h"
#include "textureResource.h"

namespace luna{
	LUNA_IMPLEMENT_CONCRETE(luna::Demo);

	WNDPROC Demo::mWndProc;

	Demo::Demo()
		: mWindowParam()
		, mIsPressedEsc(false)
		, mMessageBufferWrite(0)
	{
	}

	void Demo::initialize()
	{
		LUNA_TRACELINE(L"demo started.");

		if (File::exist(L"demo.pak")){
			File::readPackFile(L"demo.pak");
		}

		wstring titleStr;
		string musicStr;
		string shaderLibStr;
		string sceneLibStr;

		// グローバル定義のロード
		mResourceDemoPtr = ResourceManager::instance().load<ResourceLua>(L"data/demo.lua");
		if (mResourceDemoPtr && mResourceDemoPtr->isValid()){
			auto& L = mResourceDemoPtr->getContext();

			{
				lua_getglobal(L, "Name");
				const c8* nameStr = lua_tostring(L, -1);
				c16 wNameStr[256];
				swprintf(wNameStr, L"%hs", nameStr);
				titleStr = wNameStr;
				titleStr += L" - " __LPREFIX(__DATE__) L" " __LPREFIX(__TIME__);
			}
			{
				lua_getglobal(L, "Music");
				musicStr = lua_tostring(L, -1);
			}
			{
				lua_getglobal(L, "ShaderLib");
				shaderLibStr = lua_tostring(L, -1);
			}
			{
				lua_getglobal(L, "SceneLib");
				sceneLibStr = lua_tostring(L, -1);
			}
		}

		mWindowParam = Window::WindowParam(SettingsManager::instance().getWidth(), SettingsManager::instance().getHeight(), titleStr, SettingsManager::instance().isFullScreen());
		{
			mhCreateWindow = CreateEvent(NULL, FALSE, FALSE, NULL);
			mhCreateWindowDone = CreateEvent(NULL, FALSE, FALSE, NULL);
			mhMessageLoop = CreateEvent(NULL, FALSE, FALSE, NULL);
			mhMessageLoopDone = CreateEvent(NULL, TRUE, FALSE, NULL);

			mhWindowThread = CreateThread(NULL, 0, static_cast<LPTHREAD_START_ROUTINE>(threadProcWindow), this, 0, &mWindowThreadId);

			// Window作成
			SetEvent(mhCreateWindow);
			WaitForSingleObject(mhCreateWindowDone, INFINITE);

			// メッセージループ開始
			SetEvent(mhMessageLoop);
		}

		LUNA_TRACECODE(mGraphicsDevicePtr = make_unique<GraphicsDevice>(mWindowPtr->getContext().handle, SettingsManager::instance().getWidth(), SettingsManager::instance().getHeight(), SettingsManager::instance().isFullScreen()));
		LUNA_TRACECODE(SoundManager::instance().initialize(mWindowPtr->getContext().handle, musicStr.c_str()));

		// 定義済みシェーダーリソースのロード
		std::unordered_map<std::wstring, ShaderResource*> shaderMap;

		mResourceShaderLibPtr = ResourceManager::instance().load<ResourceLua>(shaderLibStr.c_str());
		if (mResourceShaderLibPtr && mResourceShaderLibPtr->isValid()){
			auto& L = mResourceShaderLibPtr->getContext();

			lua_getglobal(L, "Shaders");
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				auto const* fileName = lua_tostring(L, -1);
				if (auto shader = ResourceManager::instance().load<ShaderResource>(fileName)) {
					if (shader->isValid()) {
						shaderMap.insert(std::make_pair(shader->getName(), shader));
					}
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}

		LUNA_TRACECODE(mRendererPtr = make_unique<Renderer>(mGraphicsDevicePtr->getDevice(), mGraphicsDevicePtr->getDeviceContext(), std::move(shaderMap),
			mWindowPtr->getContext().width, mWindowPtr->getContext().height));

		// シーン定義のロード
		mResourceSceneLibPtr = ResourceManager::instance().load<ResourceLua>(sceneLibStr.c_str());
		if (mResourceSceneLibPtr && mResourceSceneLibPtr->isValid()){
			auto& L = mResourceSceneLibPtr->getContext();

			lua_getglobal(L, "Scenes");
			lua_pushnil(L);
			while (lua_next(L, -2)) {
				auto const* fileName = lua_tostring(L, -1);
				auto resourceScenePtr = ResourceManager::instance().load<ResourceLua>(fileName);
				mResourceScenePtrTbl.push_back(resourceScenePtr);
				if (resourceScenePtr && resourceScenePtr->isValid()) {
					auto& L = resourceScenePtr->getContext();

					const c8* nameStr = "null";
					f32 start = 0.f, end = 0.f;
					{
						lua_getglobal(L, "Name");
						nameStr = lua_tostring(L, -1);
						lua_pop(L, 1);
					}
					{
						lua_getglobal(L, "Start");
						start = (f32)lua_tonumber(L, -1);
						lua_pop(L, 1);
					}
					{
						lua_getglobal(L, "End");
						end = (f32)lua_tonumber(L, -1);
						lua_pop(L, 1);
					}

					c16 wNameStr[256];
					swprintf(wNameStr, L"%hs", nameStr);
					auto scenePtr = SceneManager::instance().pushScene(wNameStr, start, end);
					if (scenePtr){
						lua_getglobal(L, "Tasks");
						lua_pushnil(L);
						while (lua_next(L, -2)) {
							lua_pushnil(L);

							const c8* type = "";
							const c8* name = "";

							while (lua_next(L, -2)){
								auto const* a = lua_tostring(L, -2);
								if (strcmp(a, "Type") == 0){
									type = lua_tostring(L, -1);
								}
								else if (strcmp(a, "Name") == 0){
									name = lua_tostring(L, -1);
								}
								lua_pop(L, 1);
							}

							c16 wNameStr[256];
							swprintf(wNameStr, L"%hs", name);
							auto* tiPtr = TypeInfo::findTypeInfo(type);
							if (!tiPtr){
								tiPtr = TypeInfo::findTypeInfo((std::string("luna::") + type).c_str());
								if (!tiPtr){
									LUNA_ERRORLINE(L"type %hs not found.", type);
									//									LUNA_ASSERT(tiPtr, L"type not found.");
								}
							}
							if (tiPtr){
								scenePtr->createTask(*tiPtr, wNameStr);
							}

							lua_pop(L, 1);
						}
						lua_pop(L, 1);
					}
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}

		//HACK: party version
		{
			const luna::TypeInfo* tiTbl[] = 
			{
				TypeInfo::findTypeInfo("luna::GenericPostEffectTask"),
				TypeInfo::findTypeInfo("luna::ScreenCaptureTask"),
				//TypeInfo::findTypeInfo("luna::AnimationCameraTask"),
			};

			const auto& tbl = SceneManager::instance().getSceneTbl();
			for (auto& scene : tbl){
				for (auto& ti : tiTbl){
					if (!ti){
						continue;
					}
					if (!scene->hasTaskOfType(*ti)){
						scene->createTask(*ti, L"autogen");
					}
				}
			}
		}

		// 画面フェード用
		{
			D3D11_BUFFER_DESC desc;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.ByteWidth = sizeof(ExitFade);
			desc.CPUAccessFlags = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;
			auto hr = mRendererPtr->getContext().mDevice.CreateBuffer(&desc, nullptr, mExitFadeCB.GetAddressOf());
			if (FAILED(hr)){
				LUNA_ERROR(L"ID3D11Device::CreateBuffer");
				LUNA_ASSERT(0, L"");
			}
		}

		// ローダー
		{
			D3D11_BUFFER_DESC desc;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.ByteWidth = sizeof(Loader);
			desc.CPUAccessFlags = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 0;
			auto hr = mRendererPtr->getContext().mDevice.CreateBuffer(&desc, nullptr, mLoaderCB.GetAddressOf());
			if (FAILED(hr)){
				LUNA_ERROR(L"ID3D11Device::CreateBuffer");
				LUNA_ASSERT(0, L"");
			}
		}
		mLoaderTexturePtr = ResourceManager::instance().load<TextureResource>("data/texture/loader.dds");
		LUNA_TRACECODE(ResourceManager::instance().readPackFile(L"demo.pak", [&](f32 progress)
		{
			LUNA_TRACELINE(L"Load progress: %.3f", progress);
			doLoadScene(progress);
		}));
	}

	void Demo::doLoadScene(f32 progress)
	{
		mGraphicsDevicePtr->clear();
		mRendererPtr->execute();

		auto& context = GraphicsDevice::instance().getDeviceContext();

		ID3D11RenderTargetView* rtvTbl[] = { &GraphicsDevice::instance().getBackBuffer() };
		context.OMSetRenderTargets(1, rtvTbl, &GraphicsDevice::instance().getDepthStencil());

		f32 color[] = { 0.0f, 0.0f, 0.0f, 1.f };
		context.ClearRenderTargetView(&GraphicsDevice::instance().getBackBuffer(), color);
		context.ClearDepthStencilView(&GraphicsDevice::instance().getDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		auto techniquePtr = Renderer::instance().getContext().mShaderMap[L"posteffect"]->findTechnique("loader");

		context.IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
		context.IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
		context.IASetInputLayout(nullptr);
		context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context.VSSetShader(techniquePtr->mVS.shaderPtr, nullptr, 0);
		context.HSSetShader(techniquePtr->mHS.shaderPtr, nullptr, 0);
		context.DSSetShader(techniquePtr->mDS.shaderPtr, nullptr, 0);
		context.GSSetShader(techniquePtr->mGS.shaderPtr, nullptr, 0);
		context.CSSetShader(techniquePtr->mCS.shaderPtr, nullptr, 0);
		context.PSSetShader(techniquePtr->mPS.shaderPtr, nullptr, 0);

		ID3D11ShaderResourceView* srvTbl[] = { &mLoaderTexturePtr->getSRV() };
		context.PSSetShaderResources(0, 1, srvTbl);

		mLoader.progress = XMFLOAT4(progress, progress, progress, progress);
		mLoader.aspectH = (f32)SettingsManager::instance().getAspectH();
		mLoader.aspectV = (f32)SettingsManager::instance().getAspectV();
		context.UpdateSubresource(mLoaderCB.Get(), 0, nullptr, &mLoader, 0, 0);
		ID3D11Buffer* cbTbl[]{ mLoaderCB.Get() };
		context.PSSetConstantBuffers(1, 1, cbTbl);

		context.Draw(3, 0);

		{
			ID3D11ShaderResourceView* srvTbl[] = { nullptr };
			context.PSSetShaderResources(0, 1, srvTbl);
		}

		context.VSSetShader(nullptr, nullptr, 0);
		context.HSSetShader(nullptr, nullptr, 0);
		context.DSSetShader(nullptr, nullptr, 0);
		context.GSSetShader(nullptr, nullptr, 0);
		context.CSSetShader(nullptr, nullptr, 0);
		context.PSSetShader(nullptr, nullptr, 0);

		mGraphicsDevicePtr->present();
	}

	bool Demo::run()
	{
		dispatchMessage();

#if !LUNA_PUBLISH
		if (mResourceDemoPtr->isReloaded() || mResourceShaderLibPtr->isReloaded() || mResourceSceneLibPtr->isReloaded() || any_of(mResourceScenePtrTbl.begin(), mResourceScenePtrTbl.end(), [&](const ResourceLua* resourceScenePtr){ return resourceScenePtr->isReloaded(); })){
			doReboot(false);
		}
#endif

		if (!SceneManager::instance().isReady()){
			if (SceneManager::instance().load()){
				SceneManager::instance().fixup();
			}

			doLoadScene(1.f);
		}else{
			mGraphicsDevicePtr->clear();
			SceneManager::instance().run();
			mRendererPtr->execute();

			if (isPressedEsc()){
				if (!doExitFade()){
					setRunnable(false);
				}
			}

			mGraphicsDevicePtr->present();

			{// we need to perform this at least after 1 frame processed; flags such as mFirstUpdate sucks, really.
#if !LUNA_PUBLISH
				static bool isDispatchedCommandLine = false;
				if (!isDispatchedCommandLine){
					isDispatchedCommandLine = true;

					int argc = __argc;
					c8** argv = __argv;
					for (int i = 0; i < argc; ++i){
						if (strcmp(argv[i], "--startFrom") == 0){
							const f32 second = (f32)atof(argv[i + 1]);
							SceneManager::instance().onControlJump(second);
						}
					}
				}
#endif
			}
		}

		return !mWindowPtr->getContext().destroyed;
	}

	void Demo::finalize()
	{
		mRendererPtr.reset();
		mGraphicsDevicePtr.reset();

		while (!isExitMessageLoop()){
			Sleep(1);
		}
		WaitForSingleObject(mhWindowThread, INFINITE);
		CloseHandle(mhWindowThread);

		LUNA_TRACELINE(L"demo exited.");
	}

	bool Demo::doExitFade()
	{
		static DWORD dwStartTime = timeGetTime();
		const float fVol = 1.f;
		const DWORD dwFadeTime = 3000;//5000;

		DWORD dwElapsedTime = timeGetTime() - dwStartTime;

		float fRatio = 1.0f - (dwElapsedTime / (float)dwFadeTime);//1 to 0

		if (fRatio < 0.0f){//fRatio>1.0f){
			fRatio = 0.f;
		}

		//Pow2
		{
			float alphaf = fRatio*fRatio;
			mExitFade.color = XMFLOAT4(0, 0, 0, 1.0f * (1.0f - alphaf));
			if (fRatio != 0.0f){
				fRatio = sqrtf(fRatio);
			}
			SoundManager::instance().setVolume(fVol*fRatio);
		}

		//フェードスクリーン
		{
			ID3D11RenderTargetView* rtvTbl[] = { &GraphicsDevice::instance().getBackBuffer() };
			GraphicsDevice::instance().getDeviceContext().OMSetRenderTargets(1, rtvTbl, &GraphicsDevice::instance().getDepthStencil());

			auto techniquePtr = mRendererPtr->getContext().mShaderMap[L"posteffect"]->findTechnique("exitFade");

			auto& context = mRendererPtr->getContext().mContext;
			context.IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
			context.IASetIndexBuffer(nullptr, (DXGI_FORMAT)0, 0);
			context.IASetInputLayout(nullptr);
			context.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			context.VSSetShader(techniquePtr->mVS.shaderPtr, nullptr, 0);
			context.HSSetShader(techniquePtr->mHS.shaderPtr, nullptr, 0);
			context.DSSetShader(techniquePtr->mDS.shaderPtr, nullptr, 0);
			context.GSSetShader(techniquePtr->mGS.shaderPtr, nullptr, 0);
			context.CSSetShader(techniquePtr->mCS.shaderPtr, nullptr, 0);
			context.PSSetShader(techniquePtr->mPS.shaderPtr, nullptr, 0);

			context.UpdateSubresource(mExitFadeCB.Get(), 0, nullptr, &mExitFade, 0, 0);
			ID3D11Buffer* cbTbl[]{ mExitFadeCB.Get() };
			context.PSSetConstantBuffers(1, 1, cbTbl);

			GraphicsDevice::instance().getDeviceContext().OMSetBlendState(mRendererPtr->getContext().mBlendStateTbl[BlendStateType_Alpha].getBlendState().Get(), 0, 0xffffffff);
			context.Draw(3, 0);

			context.VSSetShader(nullptr, nullptr, 0);
			context.HSSetShader(nullptr, nullptr, 0);
			context.DSSetShader(nullptr, nullptr, 0);
			context.GSSetShader(nullptr, nullptr, 0);
			context.CSSetShader(nullptr, nullptr, 0);
			context.PSSetShader(nullptr, nullptr, 0);
		}

		return fRatio > 0.00001f;
	}

	void Demo::doReboot(bool preservePosition)
	{
#if !LUNA_PUBLISH
		//if (::MessageBox(mWindowPtr->getContext().handle, L"システム系のリソースが更新されました。\n再起動する？", L"dev", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST) == IDYES){
		c16 path[1024] = {};
		GetModuleFileName(GetModuleHandle(nullptr), path, sizeof(mResourceDemoPtr) / sizeof(*mResourceDemoPtr) - 1);
		c16 dir[1024] = {};
		GetCurrentDirectory(sizeof(dir) / sizeof(*dir) - 1, dir);
		c16 args[1024] = {};
		if (preservePosition){
			swprintf(args, L"--startFrom %f", SceneManager::instance().getElapsedSecond());
		}
		ShellExecute(nullptr, L"open", path, args, dir, SW_SHOWNORMAL);
		ExitProcess(0);
		//}
#endif
	}

	LRESULT CALLBACK Demo::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		auto* demoPtr = static_cast<Demo*>(GetProp(hWnd, L"DemoApp"));

		switch (message)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT paint = { 0 };
			HDC hDC = BeginPaint(hWnd, &paint);
			EndPaint(hWnd, &paint);

			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}break;
		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_F4){
				demoPtr->setRunnable(false);
			}
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case WM_KEYDOWN:
		{
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case WM_KEYUP:
		{
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case WM_LBUTTONDOWN:
		{
			if (!SettingsManager::instance().isFullScreen()){
				ReleaseCapture();
				SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
			}
		}// fall-through
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case WM_MOUSEMOVE:
		{
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case 0x20A://WM_MOUSEWHELL:
		{
			demoPtr->pushMessage(hWnd, message, wParam, lParam);
		}
		break;

		case WM_ACTIVATE:// ウインドウのアクティブ状態が変化
		{
		}
		break;

		default:
			return CallWindowProc(demoPtr->mWndProc, hWnd, message, wParam, lParam);
		}
		return 0;
	}

	void Demo::pushMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		mMessage[mMessageBufferWrite].push_back(message(hWnd, msg, wParam, lParam));
	}

	void Demo::dispatchMessage()
	{
		for (vector< message >::iterator it = mMessage[mMessageBufferWrite ^ 1].begin(); it != mMessage[mMessageBufferWrite ^ 1].end(); ++it){
			HWND hWnd = it->hWnd;
			UINT message = it->msg;
			WPARAM wParam = it->wParam;
			LPARAM lParam = it->lParam;

			switch (message)
			{
			case WM_KEYDOWN:
			{
			}
			break;

			case WM_KEYUP:
			{
				evKeyUp(wParam);
			}
			break;

			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			{
				const s32 button = (message == WM_LBUTTONDOWN) ? MB_L : (message == WM_RBUTTONDOWN) ? MB_R : MB_M;
				evMouseDown((s32)((s16)LOWORD(lParam)), (s32)((s16)HIWORD(lParam)), button);
			}
			break;

			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			{
				const s32 button = (message == WM_LBUTTONUP) ? MB_L : (message == WM_RBUTTONUP) ? MB_R : MB_M;
				evMouseUp((s32)((s16)LOWORD(lParam)), (s32)((s16)HIWORD(lParam)), button);
			}
			break;

			case WM_MOUSEMOVE:
			{
				evMouseMove((s32)((s16)LOWORD(lParam)), (s32)((s16)HIWORD(lParam)));
			}
			break;

			case 0x20A://WM_MOUSEWHELL:
			{
				evMouseWheel((s32)((s16)HIWORD(wParam)));
			}
			break;
			}
		}

		mMessage[mMessageBufferWrite ^ 1].clear();
		mMessageBufferWrite ^= 1;
	}

	void Demo::doMessageLoop()
	{
		MSG msg;

		while (isRunnable()){
			const BOOL isOK = GetMessage(&msg, NULL, 0, 0);
			if (isOK == 0 || isOK == -1){
				break;
			}
			mExitCode = static_cast<s32>(msg.wParam);

			if (msg.message == WM_QUIT){
				break;
			}
			if ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_ESCAPE)){
				if (isPressedEsc()){
					break;
				}
				else{
					setPressedEsc(true);
				}
			}
			if (msg.message == WM_NULL){
				continue;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		setRunnable(false);
	}

	DWORD WINAPI Demo::threadProcWindow(void* paramPtr)
	{
		auto* demoPtr = static_cast<Demo*>(paramPtr);

		WaitForSingleObject(demoPtr->mhCreateWindow, INFINITE);
		demoPtr->mWindowPtr = unique_ptr<Window>(Window::create(demoPtr->mWindowParam));
		SetProp(demoPtr->mWindowPtr->getContext().handle, L"DemoApp", demoPtr);
		demoPtr->mWndProc = (WNDPROC)SetWindowLongPtr(demoPtr->mWindowPtr->getContext().handle, GWLP_WNDPROC, (LONG)Demo::WndProc);
		SetEvent(demoPtr->mhCreateWindowDone);

		WaitForSingleObject(demoPtr->mhMessageLoop, INFINITE);
		demoPtr->doMessageLoop();
		SetEvent(demoPtr->mhMessageLoopDone);

		return TRUE;
	}

	bool Demo::isExitMessageLoop()
	{
		return WaitForSingleObject(mhMessageLoopDone, 0) == WAIT_OBJECT_0 ? true : false;
	}

	void Demo::exitMessageLoop()
	{
		PostThreadMessage(mWindowThreadId, WM_QUIT, 0, 0);
	}

	void Demo::evMouseUp(s32 x, s32 y, s32 b)
	{
	}

	void Demo::evMouseDown(s32 x, s32 y, s32 b)
	{
	}

	void Demo::evMouseMove(s32 x, s32 y)
	{
	}

	void Demo::evMouseWheel(s32 moveDelta)
	{
		if (moveDelta>0)
		{
			sceneControlFastForward(0.1f);
		}
		else
		{
			sceneControlRewind(0.1f);
		}
	}

	void Demo::evKeyUp(u32 key)
	{
#if !LUNA_PUBLISH
		switch (key)
		{
		// ---------------------------------------------
		// 巻き戻し
		// ---------------------------------------------
		case 'B':
		case VK_LEFT:
		case VK_F2:
		{
			sceneControlRewind(1.f);
		}break;

		// ---------------------------------------------
		// 早送り
		// ---------------------------------------------
		case 'F':
		case VK_RIGHT:
		case VK_F3:
		{
			sceneControlFastForward(1.f);
		}break;

		// 前のシーンへ
		case VK_F1:
		{
			const auto& sceneTbl = SceneManager::instance().getSceneTbl();
			for (size_t i = 0; i < sceneTbl.size(); ++i){
				auto& scene = sceneTbl[i];

				if (scene->isRunnable(SceneManager::instance().getElapsedSecond())){
					if (scene->getElapsedSecond() > 1){
						sceneControlJump(scene->getStartSecond()*1000.f);
					}
					else{
						if (i == 0){
							sceneControlJump(scene->getStartSecond()*1000.f);
						}
						else{
							sceneControlJump(sceneTbl[i - 1]->getStartSecond()*1000.f);
						}
					}
					break;
				}
			}
		}
		break;

		// 次のシーンへ
		case VK_F4:
		{
			const auto& sceneTbl = SceneManager::instance().getSceneTbl();
			for (size_t i = 0; i < sceneTbl.size(); ++i){
				auto& scene = sceneTbl[i];

				if (scene->isRunnable(SceneManager::instance().getElapsedSecond())){
					if (i == sceneTbl.size()-1){
						sceneControlJump(sceneTbl[0]->getStartSecond()*1000.f);
					}
					else{
						sceneControlJump(sceneTbl[i+1]->getStartSecond()*1000.f);
					}
					break;
				}
			}
		}
		break;

		// ---------------------------------------------
		// TopMost
		// ---------------------------------------------
		case 'T':
		{
			if (GetWindowLong(mWindowPtr->getContext().handle, GWL_EXSTYLE) & WS_EX_TOPMOST){
				SetWindowPos(mWindowPtr->getContext().handle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING);
			}
			else{
				SetWindowPos(mWindowPtr->getContext().handle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_NOREPOSITION | SWP_NOSENDCHANGING);
			}
		}break;

		// ---------------------------------------------
		// ポーズ
		// ---------------------------------------------
		case VK_SPACE:
		case 'P':
		{
			if (SceneManager::instance().isPaused()){
				SceneManager::instance().play();
			}
			else{
				SceneManager::instance().pause();
			}
		}break;

		// ---------------------------------------------
		// ミュート
		// ---------------------------------------------
		case 'M': {
		}break;

		case VK_F5:{
			if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 0x8000){
				if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0x8000){
					doReboot(false);
				}
				else{
					doReboot(true);
				}
			}
		}break;

		// ---------------------------------------------
		// ループの設定
		// ---------------------------------------------
		case 'L': {
			SettingsManager::instance().setLoopDemo(!SettingsManager::instance().isLoopDemo());
		}break;

		case VK_F8:{
			ResourceManager::instance().writePackFile(L"demo.pak");
			ResourceManager::instance().writePackFile(L"..\\prod\\demo.pak");
		}

		default:
			break;
		}
#endif
	}

	void Demo::sceneControlJump(f32 whereMSec)
	{
		SceneManager::instance().onControlJump(whereMSec / 1000.f);
	}

	void Demo::sceneControlFastForward(f32 rate)
	{
		const u32 speedBias = ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0x8000 || (GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0x8000) ? 5 : 1;
		const u32 stepTime = (u32)(SeekStepTime*speedBias * rate);

		const u32 curTime = SoundManager::instance().getCurrentTime();
		u32 newTime = (curTime / stepTime + 1) * stepTime;
		if (newTime > SoundManager::instance().getDuration()){
			newTime = curTime;
		}

		sceneControlJump((f32)newTime);
	}

	void Demo::sceneControlRewind(f32 rate)
	{
		const u32 speedBias = ((GetAsyncKeyState(VK_LSHIFT) & 0x8000) == 0x8000 || (GetAsyncKeyState(VK_RSHIFT) & 0x8000) == 0x8000) ? 5 : 1;
		const u32 stepTime = (u32)(SeekStepTime*speedBias * rate);

		const u32 curTime = SoundManager::instance().getCurrentTime();
		u32 newTime = (curTime / stepTime) * stepTime;

		if (curTime < newTime + SeekBackOffset){
			if (newTime >= stepTime){
				newTime -= stepTime;
			}
		}

		sceneControlJump((f32)newTime);
	}
}