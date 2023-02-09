#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "shlwapi")

#include <windows.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "resource.h"

WCHAR szClassName[] = L"Window";

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton1;
	static HWND hStatic1;
	static HWND hEdit1;
	static HWND hCombo1;

	static HWND hStatic2;
	static HWND hEdit2;
	static HWND hButton2;

	static HWND hStatic3;
	static HWND hEdit3;

	static TCHAR szINIFilePath[MAX_PATH]; // 居にファイル
	static WCHAR szFileNameTemplate[MAX_PATH] = L"%Y_%M_%D_%h_%m_%s";
	static DWORD dwFileType = 0; // 0:png 1:jpg 2:gif 3:bmp 4:webp
	static WCHAR szSaveDirPath[MAX_PATH];
	static DWORD dwTimeSpan = 5;

	switch (msg)
	{
	case WM_CREATE:
		{
			// iniファイルからstatic変数に格納する
			GetModuleFileName(((LPCREATESTRUCT)lParam)->hInstance, szINIFilePath, MAX_PATH);
			PathRemoveExtension(szINIFilePath);
			PathAddExtension(szINIFilePath, L".ini");

			GetPrivateProfileString(
				L"AUTO_CAPTURE",
				L"FILE_NAME_TEMP",
				L"%Y_%M_%D_%h_%m_%s",
				szFileNameTemplate,
				MAX_PATH,
				szINIFilePath
			);

			dwFileType = GetPrivateProfileInt(
				L"AUTO_CAPTURE",
				L"FILE_TYPE",
				0,
				szINIFilePath
			);

			GetPrivateProfileString(
				L"AUTO_CAPTURE",
				L"SAVE_DIR",
				L"",
				szSaveDirPath,
				MAX_PATH,
				szINIFilePath
			);

			dwTimeSpan = GetPrivateProfileInt(
				L"AUTO_CAPTURE",
				L"TIME_SPAN",
				5,
				szINIFilePath
			);

			hButton1 = CreateWindow(L"BUTTON", L"最小化してキャプチャー実行", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);

			hStatic1 = CreateWindowEx(0, L"STATIC", L"ファイル名：", WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szFileNameTemplate, WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hCombo1 = CreateWindowEx(0, L"COMBOBOX", 0, WS_VISIBLE | WS_CHILD | WS_TABSTOP | CBS_DROPDOWNLIST, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			SendMessage(hCombo1, CB_ADDSTRING, 0, (LPARAM)L".png");
			SendMessage(hCombo1, CB_ADDSTRING, 0, (LPARAM)L".jpg");
			SendMessage(hCombo1, CB_ADDSTRING, 0, (LPARAM)L".gif");
			SendMessage(hCombo1, CB_ADDSTRING, 0, (LPARAM)L".bmp");
			SendMessage(hCombo1, CB_ADDSTRING, 0, (LPARAM)L".webp");
			SendMessage(hCombo1, CB_SETCURSEL, (0 <= dwFileType && dwFileType < 5) ? dwFileType : 0, 0);

			hStatic2 = CreateWindowEx(0, L"STATIC", L"保存先：", WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szSaveDirPath, WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hButton2 = CreateWindow(L"BUTTON", L"...", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)1000, ((LPCREATESTRUCT)lParam)->hInstance, 0);

			hStatic3 = CreateWindowEx(0, L"STATIC", L"保存間隔（秒）：", WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			WCHAR szTemp[128];
			wsprintf(szTemp, L"%d", dwTimeSpan);
			hEdit3 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szTemp, WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		}
		break;
	case WM_SIZE:
		MoveWindow(hButton1, 10, 10, 256, 32, TRUE);

		MoveWindow(hStatic1, 10, 50, 128, 27, TRUE);
		MoveWindow(hEdit1, 138, 50, 256, 26, TRUE);
		MoveWindow(hCombo1, 138+256, 50, 64, 27, TRUE);

		MoveWindow(hStatic2, 10, 90, 128, 27, TRUE);
		MoveWindow(hEdit2, 138, 90, 256, 26, TRUE);
		MoveWindow(hButton2, 138 + 256, 90, 64, 27, TRUE);

		MoveWindow(hStatic3, 10, 130, 128, 27, TRUE);
		MoveWindow(hEdit3, 138, 130, 256, 26, TRUE);

		break;
	case WM_TIMER:
		{
			// static変数の内容に従いキャプチャーを実行
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			// 入力チェック
			// static変数に代入
			GetWindowText(hEdit1, szFileNameTemplate, MAX_PATH);
			dwFileType = (DWORD)SendMessage(hCombo1, CB_GETCURSEL, 0, 0);
			GetWindowText(hEdit2, szSaveDirPath, MAX_PATH);
			dwTimeSpan = (DWORD)GetDlgItemInt(hWnd, 1001, 0, 0);			
			// iniファイル保存
			WritePrivateProfileString(
				L"AUTO_CAPTURE",
				L"FILE_NAME_TEMP",
				szFileNameTemplate,
				szINIFilePath
			);
			WCHAR szTemp[128];
			wsprintf(szTemp, L"%d", dwFileType);
			WritePrivateProfileString(
				L"AUTO_CAPTURE",
				L"FILE_TYPE",
				szTemp,
				szINIFilePath
			);
			WritePrivateProfileString(
				L"AUTO_CAPTURE",
				L"SAVE_DIR",
				szSaveDirPath,
				szINIFilePath
			);
			wsprintf(szTemp, L"%d", dwTimeSpan);
			WritePrivateProfileString(
				L"AUTO_CAPTURE",
				L"TIME_SPAN",
				szTemp,
				szINIFilePath
			);
			// ウィンドウ非表示

			// タイマースタート

		} else if (LOWORD(wParam) == 1000) {
			BROWSEINFO  bi = { 0 };
			bi.hwndOwner = hWnd;
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			bi.lpszTitle = TEXT("フォルダを選択してください。");
			LPITEMIDLIST pidl = (LPITEMIDLIST)SHBrowseForFolder(&bi);
			LPMALLOC pMalloc = 0;
			if (pidl != NULL && SHGetMalloc(&pMalloc) != E_FAIL)
			{
				TCHAR szDirectoryPath[MAX_PATH];
				SHGetPathFromIDList(pidl, szDirectoryPath);
				SetWindowText(hEdit2, szDirectoryPath);
				SendMessage(hEdit2, EM_SETSEL, 0, -1);
				SetFocus(hEdit2);
				pMalloc->Free(pidl);
				pMalloc->Release();
			} else {
				SetWindowText(hEdit2, 0);
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, 0x1234);
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nShowCmd
) {
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		L"auto capture",
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
