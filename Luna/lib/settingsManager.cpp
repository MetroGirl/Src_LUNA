//
// SettingsManager.cpp
//

#include "stdafx.h"
#include "settingsManager.h"
#include "resource.h"

namespace luna{
#if LUNA_PUBLISH
	SettingsManager::DemoSettings SettingsManager::defaultSettings = {
		800, 600,
		60,//	Hz
		32,
		16, 9,
		true,
		true,
		2,
		false,
		false,
		false,
		false,
		false
	};
#else
	SettingsManager::DemoSettings SettingsManager::defaultSettings = {
		1280, 720,// Resolution
		60,//	Hz
		32,//	Bpp
		16, 9,// Aspect Ratio
		false,// FullScreen
		true,// Vsync
		0,		// MSAA sampl count.
		false,// AlwaysOnTop
		false,// NoSound
		false,// BigScreenMode
		false,// LowQuality
		false// LoopDemo
	};
#endif

	LUNA_IMPLEMENT_CONCRETE(luna::SettingsManager);
	LUNA_IMPLEMENT_SINGLETON(luna::SettingsManager);

	bool SettingsManager::initialize()
	{
#if LUNA_PUBLISH
		return (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG), NULL, (DLGPROC)SettingsManager::DlgProc) == IDOK);
#else
		mRunSettings = defaultSettings;
		return true;
#endif
	}

	LRESULT CALLBACK SettingsManager::DlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
	{
		static const AspectRatio gAspectDef[] = {
			{ 4, 3 }, { 16, 9 }, { 16, 10 }, { 5, 4 }
		};
		static const int gSampleDef[] = { 0, 2, 4, 8 };

		switch (msg)
		{
			// ���������
		case WM_INITDIALOG:
		{
			//-------------------------------------------
			// �_�C�A���O���f�X�N�g�b�v�̐^�񒆂Ɉړ�
			//-------------------------------------------
			RECT deskrc, rc;
			GetWindowRect(GetDesktopWindow(), reinterpret_cast<LPRECT>(&deskrc));
			GetWindowRect(hDlg, reinterpret_cast<LPRECT>(&rc));
			LONG x = (deskrc.right - (rc.right - rc.left)) / 2;
			LONG y = (deskrc.bottom - (rc.bottom - rc.top)) / 2;
			SetWindowPos(hDlg, HWND_TOP, x, y, (rc.right - rc.left), (rc.bottom - rc.top), SWP_SHOWWINDOW);


			//----------------------------
			// �𑜓x
			//----------------------------
			DEVMODE devMode;

			// �ݒ�\�ȉ𑜓x���f�o�C�X����擾���A���X�g�{�b�N�X�ɏ����ǉ����Ă���
			for (s32 i = 0;; ++i) {
				if (!EnumDisplaySettings(NULL, i, &devMode)){
					break;
				}
				if (devMode.dmBitsPerPel != SettingsManager::instance().defaultSettings.nBpp) continue;

				// ���ɓ����𑜓x�̐ݒ��o�^���Ă��Ȃ����m�F
				bool isNewReso = true;
				for (vector<Resolution>::iterator it = SettingsManager::instance().mVecResolution.begin(); it != SettingsManager::instance().mVecResolution.end(); ++it){
					if (it->w == devMode.dmPelsWidth && it->h == devMode.dmPelsHeight && it->hz==devMode.dmDisplayFrequency){
						isNewReso = false;
						break;
					}
				}
				// �V���ȉ𑜓x��ǉ�
				if (isNewReso) {
					SettingsManager::instance().mVecResolution.push_back(Resolution(devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency));
				}
			}

			// �Ȃ�EnumDisplaySettings�Ŏ擾�ł���𑜓x�̏��Ԃ��o���o���Ȃ̂ŁA
			// �����̉𑜓x�Ń\�[�g����
			std::stable_sort(SettingsManager::instance().mVecResolution.begin(), SettingsManager::instance().mVecResolution.end());

			// ���X�g�{�b�N�X�ɓo�^����
			for (u32 i = 0; i < SettingsManager::instance().mVecResolution.size(); ++i){
				c16 temp[64];
				swprintf(temp, L"%d x %d@%dhz", SettingsManager::instance().mVecResolution[i].w, SettingsManager::instance().mVecResolution[i].h, SettingsManager::instance().mVecResolution[i].hz);
				SendDlgItemMessage(hDlg, IDC_RESOLUTION, LB_INSERTSTRING, (WPARAM)i, (LPARAM)temp);
			}

			// �����ݒ�l�ɃJ�[�\����ݒ�
			// mgInitVal�Ŏw�肵���ݒ肪���݂��Ȃ��������ׂ̈ɂƂ肠���� 0 �ԂɃJ�[�\�����w�肵�Ă���
			SendDlgItemMessage(hDlg, IDC_RESOLUTION, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			// mgInitVal ���Q�Ƃ��A�����ݒ�l�ɃJ�[�\����ݒ�
			for (u32 i = 0; i < SettingsManager::instance().mVecResolution.size(); ++i){
				if (SettingsManager::instance().mVecResolution[i].w == SettingsManager::instance().defaultSettings.nWidth && SettingsManager::instance().mVecResolution[i].h == SettingsManager::instance().defaultSettings.nHeight){
					SendDlgItemMessage(hDlg, IDC_RESOLUTION, LB_SETCURSEL, (WPARAM)i, (LPARAM)0);
				}
			}
			// �f�X�N�g�b�v�̉𑜓x�ɃZ�b�g����. ���݂��Ȃ��ꍇ�͍ł��߂����̂�I��
			HWND hDesktopWindow = GetDesktopWindow();
			HDC hDesktopDC = GetDC(hDesktopWindow);
			const s32 deskWidth = GetDeviceCaps(hDesktopDC, HORZRES);
			const s32 deskHeight = GetDeviceCaps(hDesktopDC, VERTRES);
			ReleaseDC(hDesktopWindow, hDesktopDC);

			bool isFound = false;
			for (u32 i = 0; i < SettingsManager::instance().mVecResolution.size(); ++i) {
				if (SettingsManager::instance().mVecResolution[i].w == deskWidth && SettingsManager::instance().mVecResolution[i].h == deskHeight) {
					SendDlgItemMessage(hDlg, IDC_RESOLUTION, LB_SETCURSEL, (WPARAM)i, (LPARAM)0);
					isFound = true;
				}
			}
			if (!isFound){
				SendDlgItemMessage(hDlg, IDC_RESOLUTION, LB_SETCURSEL, (WPARAM)SettingsManager::instance().mVecResolution.size() - 1, (LPARAM)0);
			}

			//----------------------------
			// �A�X�y�N�g
			//----------------------------
			// �A�X�y�N�g���`
			// ��`�ς݂̃A�X�y�N�g����A���X�g�{�b�N�X�ɏ����ǉ����Ă���
			for (s32 i = 0; i < sizeof(gAspectDef) / sizeof(*gAspectDef); ++i){
				c16 temp[64];
				s32 aspH = static_cast<s32>(gAspectDef[i].h);
				s32 aspV = static_cast<s32>(gAspectDef[i].v);
				swprintf(temp, L"%d : %d", aspH, aspV);
				SendDlgItemMessage(hDlg, IDC_ASPECT, LB_INSERTSTRING, (WPARAM)i, (LPARAM)temp);
			}

			// �����ݒ�l�ɃJ�[�\����ݒ�
			// mgInitVal�Ŏw�肵���ݒ肪���݂��Ȃ��������ׂ̈ɂƂ肠���� 0 �ԂɃJ�[�\�����w�肵�Ă���
			SendDlgItemMessage(hDlg, IDC_ASPECT, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			// mgInitVal ���Q�Ƃ��A�����ݒ�l�ɃJ�[�\����ݒ�
			for (s32 i = 0; i < sizeof(gAspectDef) / sizeof(*gAspectDef); ++i){
				if (gAspectDef[i].h == SettingsManager::instance().defaultSettings.nAspH && gAspectDef[i].v == SettingsManager::instance().defaultSettings.nAspV){
					SendDlgItemMessage(hDlg, IDC_ASPECT, LB_SETCURSEL, (WPARAM)i, (LPARAM)0);
				}
			}

			// �f�X�N�g�b�v�̉𑜓x����A�X�y�N�g����Z�o. ���݂��Ȃ��ꍇ�͉������Ȃ�(mgInitVal�𑸏d)
			f32 deskAspect = deskWidth / static_cast<f32>(deskHeight);
			s32 deskAspectW = SettingsManager::instance().defaultSettings.nAspH;
			s32 deskAspectH = SettingsManager::instance().defaultSettings.nAspV;
			if (deskAspect == 4 / 3.f){
				deskAspectW = 4;
				deskAspectH = 3;
			}
			else if (deskAspect == 5 / 4.f){
				deskAspectW = 5;
				deskAspectH = 4;
			}
			else if (deskAspect == 16 / 9.f){
				deskAspectW = 16;
				deskAspectH = 9;
			}
			else if (deskAspect == 16 / 10.f){
				deskAspectW = 16;
				deskAspectH = 10;
			}
			for (s32 i = 0; i < sizeof(gAspectDef) / sizeof(*gAspectDef); ++i){
				if (gAspectDef[i].h == deskAspectW && gAspectDef[i].v == deskAspectH) {
					SendDlgItemMessage(hDlg, IDC_ASPECT, LB_SETCURSEL, (WPARAM)i, (LPARAM)0);
				}
			}

			//----------------------------
			// �A���`�G�C���A�X
			//----------------------------
			// �A���`�G�C���A�X�T���v�����O����`
			//// �ݒ�\�ȍő�T���v�����O�����擾
			//s32 maxSampleNum = GL::GetAvailableMultiSampleNum( gSampleDef[gSampleDefNum-1] );

			// ��`�ς݂̃T���v�����̂����A�ݒ�\�Ȃ��̂��A���X�g�{�b�N�X�ɏ����ǉ����Ă���
			for (s32 i = 0; i < sizeof(gSampleDef) / sizeof(*gSampleDef); ++i) {
				c16 temp[64];
				if (i == 0) {
					swprintf(temp, L"none");
				}
				else {
					swprintf(temp, L"x %d", gSampleDef[i]);
				}

				SendDlgItemMessage(hDlg, IDC_FSAA, LB_INSERTSTRING, (WPARAM)i, (LPARAM)temp);
			}

			// �����ݒ�l�ɃJ�[�\����ݒ�
			// mgInitVal�Ŏw�肵���ݒ肪���݂��Ȃ��������ׂ̈ɂƂ肠���� 0 �ԂɃJ�[�\�����w�肵�Ă���
			SendDlgItemMessage(hDlg, IDC_FSAA, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			// mgInitVal ���Q�Ƃ��A�����ݒ�l�ɃJ�[�\����ݒ�
			for (s32 i = 0; i < sizeof(gSampleDef) / sizeof(*gSampleDef); ++i) {
				if (gSampleDef[i] == SettingsManager::instance().defaultSettings.nMsaa) {
					SendDlgItemMessage(hDlg, IDC_FSAA, LB_SETCURSEL, (WPARAM)i, (LPARAM)0);
				}
			}


			//----------------------------
			// �t���X�N���[��
			//----------------------------
			if (SettingsManager::instance().defaultSettings.isFullscreen) {
				SendDlgItemMessage(hDlg, IDC_CHECK_FULLSCREEN, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AOT), FALSE);
			}
			else {
				SendDlgItemMessage(hDlg, IDC_CHECK_FULLSCREEN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
			}

			//----------------------------
			// V Sync
			//----------------------------
			if (SettingsManager::instance().defaultSettings.isVSync) {
				SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			}
			else {
				SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
			}

			//----------------------------
			// �r�b�O�X�N���[��
			//----------------------------
			if (SettingsManager::instance().defaultSettings.isBigScreenMode) {
				SendDlgItemMessage(hDlg, IDC_CHECK_BIGSCREEN, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			}
			else {
				SendDlgItemMessage(hDlg, IDC_CHECK_BIGSCREEN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
			}

			//----------------------------
			// Always on Top
			//----------------------------
			if (SettingsManager::instance().defaultSettings.isAlwaysOnTop) {
				SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			}
			else {
				SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
			}

			//----------------------------
			// Low Quality
			//----------------------------
			if (SettingsManager::instance().defaultSettings.isLowQuality) {
				SendDlgItemMessage(hDlg, IDC_CHECK_AOT2, BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
			}
			else {
				SendDlgItemMessage(hDlg, IDC_CHECK_AOT2, BM_SETCHECK, (WPARAM)BST_UNCHECKED, (LPARAM)0);
			}

			return TRUE;
		}
		case WM_LBUTTONDOWN: {
			PostMessage(hDlg, WM_NCLBUTTONDOWN, (WPARAM)HTCAPTION, lp);
			return TRUE;
		}
		case WM_COMMAND: {
			switch (LOWORD(wp)) {
			case IDC_CHECK_FULLSCREEN:
			{
				const bool tempAotChecked = static_cast<BOOL>(SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_GETCHECK, 0, 0)) ? true : false;

				// �����ɗ������̓`�F�b�N�������s����O�Ȃ̂ŁA�`�F�b�N����Ă��Ȃ���΁A���ꂩ��`�F�b�N�����
				if (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_CHECK_FULLSCREEN, BM_GETCHECK, 0, 0)) {
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AOT), false);
					//SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_SETSTATE, (WPARAM)FALSE, 0);
				}
				else {
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_AOT), true);
					//SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_SETSTATE, (WPARAM)TRUE, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_SETCHECK, (WPARAM)tempAotChecked, 0);
				}

				// BigScreen!
				if (SendDlgItemMessage(hDlg, IDC_CHECK_FULLSCREEN, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_BIGSCREEN), TRUE);
				}
				else {
					EnableWindow(GetDlgItem(hDlg, IDC_CHECK_BIGSCREEN), FALSE);
				}
			} return TRUE;
			case IDC_CHECK_AOT2:
			{
				if (SendDlgItemMessage(hDlg, IDC_CHECK_AOT2, BM_GETCHECK, 0, 0) == BST_CHECKED) {
					EnableWindow(GetDlgItem(hDlg, IDC_FSAA), FALSE);
				}
				else {
					EnableWindow(GetDlgItem(hDlg, IDC_FSAA), TRUE);
				}
			} return TRUE;
			case IDC_RUN:
			{
				const s32 resoIndex = static_cast<s32>(SendDlgItemMessage(hDlg, IDC_RESOLUTION, LB_GETCURSEL, (WPARAM)0, (LPARAM)0));
				SettingsManager::instance().mRunSettings.nWidth = SettingsManager::instance().mVecResolution[resoIndex].w;
				SettingsManager::instance().mRunSettings.nHeight = SettingsManager::instance().mVecResolution[resoIndex].h;
				SettingsManager::instance().mRunSettings.nFrequency = SettingsManager::instance().mVecResolution[resoIndex].hz;

				const s32 aspIndex = static_cast<s32>(SendDlgItemMessage(hDlg, IDC_ASPECT, LB_GETCURSEL, (WPARAM)0, (LPARAM)0));
				SettingsManager::instance().mRunSettings.nAspH = static_cast<s32>(gAspectDef[aspIndex].h);
				SettingsManager::instance().mRunSettings.nAspV = static_cast<s32>(gAspectDef[aspIndex].v);

				const s32 aaIndex = static_cast<s32>(SendDlgItemMessage(hDlg, IDC_FSAA, LB_GETCURSEL, (WPARAM)0, (LPARAM)0));
				SettingsManager::instance().mRunSettings.isFullscreen = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_CHECK_FULLSCREEN, BM_GETCHECK, 0, 0));
				SettingsManager::instance().mRunSettings.isVSync = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_CHECK_VSYNC, BM_GETCHECK, 0, 0));
				SettingsManager::instance().mRunSettings.isNoSound = SettingsManager::instance().defaultSettings.isNoSound;
				SettingsManager::instance().mRunSettings.isBigScreenMode = SettingsManager::instance().mRunSettings.isFullscreen && (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_CHECK_BIGSCREEN, BM_GETCHECK, 0, 0));
				SettingsManager::instance().mRunSettings.isAlwaysOnTop = !SettingsManager::instance().mRunSettings.isFullscreen && (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_CHECK_AOT, BM_GETCHECK, 0, 0));
				SettingsManager::instance().mRunSettings.isLowQuality = (BST_CHECKED == SendDlgItemMessage(hDlg, IDC_CHECK_AOT2, BM_GETCHECK, 0, 0));
				SettingsManager::instance().mRunSettings.nMsaa = SettingsManager::instance().mRunSettings.isLowQuality ? 0 : gSampleDef[aaIndex];// LQ�Ȃ�AA�͖���

				// �A�X�y�N�g��̎Z�o : �t���X�N���[������DemoSettings�̂��̂����̂܂܎g��
				//										: �E�B���h�E���̓E�B���h�E�T�C�Y����Z�o����
				if (!SettingsManager::instance().mRunSettings.isFullscreen){
					const f32 aspect = SettingsManager::instance().mRunSettings.nWidth / static_cast<f32>(SettingsManager::instance().mRunSettings.nHeight);
					if (aspect == 4 / 3.f){
						SettingsManager::instance().mRunSettings.nAspH = 4;
						SettingsManager::instance().mRunSettings.nAspV = 3;
					}
					else if (aspect == 5 / 4.f){
						SettingsManager::instance().mRunSettings.nAspH = 5;
						SettingsManager::instance().mRunSettings.nAspV = 4;
					}
					else if (aspect == 16 / 9.f){
						SettingsManager::instance().mRunSettings.nAspH = 16;
						SettingsManager::instance().mRunSettings.nAspV = 9;
					}
					else if (aspect == 16 / 10.f){
						SettingsManager::instance().mRunSettings.nAspH = 16;
						SettingsManager::instance().mRunSettings.nAspV = 10;
					}
					else{
						LUNA_ERRORLINE(L"aspect not found. Using default aspect.\n");
					}
				}
				SettingsManager::instance().mRunSettings.isLoopDemo = false;

				EndDialog(hDlg, IDOK);
			} return TRUE;
			case IDC_CLOSE:
			{
				EndDialog(hDlg, IDCANCEL);

			} return TRUE;
			case IDC_WWW:
			{
				ShellExecute(hDlg, L"open", L"http://crvthecoder.github.io/Luna/", NULL, NULL, SW_NORMAL);
			}
			}
			return TRUE;
		}

		}

		return FALSE;
	}
}
