#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "shlwapi")
#pragma comment(lib, "gdiplus")

#include <windows.h>
#include <gdiplus.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <string>
#include "resource.h"

#define MYWM_NOTIFYICON (WM_APP+100)
#define TRAY_ID 1
WCHAR szClassName[] = L"Window";
using namespace Gdiplus;

int GetEncoderClsid(WCHAR* format, CLSID* pClsid)
{
	unsigned int num = 0, size = 0;
	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;
	ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL) return -1;
	GetImageEncoders(num, size, pImageCodecInfo);
	for (unsigned int j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}
	free(pImageCodecInfo);
	return -1;
}

int GetScreeny(LPWSTR lpszFilename, WCHAR* format, ULONG uQuality) // by Napalm
{
	HWND hMyWnd = GetForegroundWindow(); // get my own window
	RECT  r;                             // the area we are going to capture 
	int w, h;                            // the width and height of the area
	HDC dc;                              // the container for the area
	int nBPP;
	HDC hdcCapture;
	LPBYTE lpCapture;
	int nCapture;
	int iRes;
	CLSID imageCLSID;
	Bitmap* pScreenShot;
	// get the area of my application's window  
	//GetClientRect(hMyWnd, &r);
	GetWindowRect(hMyWnd, &r);
	dc = GetWindowDC(hMyWnd);//   GetDC(hMyWnd) ;
	w = r.right - r.left;
	h = r.bottom - r.top;
	nBPP = GetDeviceCaps(dc, BITSPIXEL);
	hdcCapture = CreateCompatibleDC(dc);
	// create the buffer for the screenshot
	BITMAPINFO bmiCapture = {
		  sizeof(BITMAPINFOHEADER), w, -h, 1, (WORD)nBPP, BI_RGB, 0, 0, 0, 0, 0,
	};
	// create a container and take the screenshot
	HBITMAP hbmCapture = CreateDIBSection(dc, &bmiCapture,
		DIB_PAL_COLORS, (LPVOID*)&lpCapture, NULL, 0);
	// failed to take it
	if (!hbmCapture)
	{
		DeleteDC(hdcCapture);
		DeleteDC(dc);
		return 0;
	}
	// copy the screenshot buffer
	nCapture = SaveDC(hdcCapture);
	SelectObject(hdcCapture, hbmCapture);
	BitBlt(hdcCapture, 0, 0, w, h, dc, 0, 0, SRCCOPY);
	RestoreDC(hdcCapture, nCapture);
	DeleteDC(hdcCapture);
	DeleteDC(dc);
	// save the buffer to a file    
	pScreenShot = new Bitmap(hbmCapture, (HPALETTE)NULL);
	EncoderParameters encoderParams;
	encoderParams.Count = 1;
	encoderParams.Parameter[0].NumberOfValues = 1;
	encoderParams.Parameter[0].Guid = EncoderQuality;
	encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
	encoderParams.Parameter[0].Value = &uQuality;
	GetEncoderClsid(format, &imageCLSID);
	iRes = (pScreenShot->Save(lpszFilename, &imageCLSID, &encoderParams) == Ok);
	delete pScreenShot;
	DeleteObject(hbmCapture);
	return iRes;
}

std::wstring ReplaceString
(
	std::wstring String1  // 置き換え対象
	, std::wstring String2  // 検索対象
	, std::wstring String3  // 置き換える内容
)
{
	std::wstring::size_type  Pos(String1.find(String2));
	while (Pos != std::string::npos)
	{
		String1.replace(Pos, String2.length(), String3);
		Pos = String1.find(String2, Pos + String3.length());
	}
	return String1;
}

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
	POINT point;
	NOTIFYICONDATA tnd;
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
			SendMessage(hCombo1, CB_SETCURSEL, (0 <= dwFileType && dwFileType < 4) ? dwFileType : 0, 0);
			hStatic2 = CreateWindowEx(0, L"STATIC", L"保存先：", WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szSaveDirPath, WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hButton2 = CreateWindow(L"BUTTON", L"...", WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)1000, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			hStatic3 = CreateWindowEx(0, L"STATIC", L"保存間隔（秒）：", WS_VISIBLE | WS_CHILD | SS_RIGHT | SS_CENTERIMAGE, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			WCHAR szTemp[128];
			wsprintf(szTemp, L"%d", dwTimeSpan);
			hEdit3 = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", szTemp, WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 0, 0, 0, 0, hWnd, (HMENU)1001, ((LPCREATESTRUCT)lParam)->hInstance, 0);
			tnd.cbSize = sizeof(NOTIFYICONDATA);
			tnd.hWnd = hWnd;
			tnd.uID = TRAY_ID;
			tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
			tnd.uCallbackMessage = MYWM_NOTIFYICON;
			tnd.hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1));
			lstrcpyn(tnd.szTip, L"ここ", sizeof(tnd.szTip));
			Shell_NotifyIcon(NIM_ADD, &tnd);
			// タイマースタート
			if (dwTimeSpan > 0) {
				SetTimer(hWnd, 0x1234, dwTimeSpan * 1000, 0);
			}
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
			WCHAR szFilePath[MAX_PATH];
			lstrcpy(szFilePath, szSaveDirPath);
			WCHAR szYear[5];
			WCHAR szMonth[3];
			WCHAR szDay[3];
			WCHAR szHour[3];
			WCHAR szMinute[3];
			WCHAR szSecond[3];
			SYSTEMTIME st = {};
			GetLocalTime(&st);
			wsprintf(szYear, L"%04d", st.wYear);
			wsprintf(szMonth, L"%02d", st.wMonth);
			wsprintf(szDay, L"%02d", st.wDay);
			wsprintf(szHour, L"%02d", st.wHour);
			wsprintf(szMinute, L"%02d", st.wMinute);
			wsprintf(szSecond, L"%02d", st.wSecond);
			std::wstring str = ReplaceString(szFileNameTemplate, L"%Y", szYear);
			str = ReplaceString(str.c_str(), L"%M", szMonth);
			str = ReplaceString(str.c_str(), L"%D", szDay);
			str = ReplaceString(str.c_str(), L"%h", szHour);
			str = ReplaceString(str.c_str(), L"%m", szMinute);
			str = ReplaceString(str.c_str(), L"%s", szSecond);
			PathAppend(szFilePath, str.c_str());
			switch (dwFileType)
			{
			case 0:
				PathAddExtension(szFilePath, L".png");
				GetScreeny(szFilePath, L"image/png", 50);
				break;
			case 1:
				PathAddExtension(szFilePath, L".jpg");
				GetScreeny(szFilePath, L"image/jpeg", 50);
				break;
			case 2:
				PathAddExtension(szFilePath, L".gif");
				GetScreeny(szFilePath, L"image/gif", 50);
				break;
			case 3:
				PathAddExtension(szFilePath, L".bmp");
				GetScreeny(szFilePath, L"image/bmp", 50);
				break;
			}

			// static変数の内容に従いキャプチャーを実行
		}
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetWindowText(hEdit1, szFileNameTemplate, MAX_PATH);
			dwFileType = (DWORD)SendMessage(hCombo1, CB_GETCURSEL, 0, 0);
			GetWindowText(hEdit2, szSaveDirPath, MAX_PATH);
			dwTimeSpan = (DWORD)GetDlgItemInt(hWnd, 1001, 0, 0);			
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
			ShowWindow(hWnd, SW_HIDE);
			SetTimer(hWnd, 0x1234, dwTimeSpan * 1000, 0);
		}
		else if (LOWORD(wParam) == 1000) {
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
			}
			else {
				SetWindowText(hEdit2, 0);
			}
		}
		else if (LOWORD(wParam) == ID_EXIT) {
			DestroyWindow(hWnd);
		}
		break;
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case MYWM_NOTIFYICON:
		switch (lParam)
		{
		case WM_LBUTTONDOWN:
			KillTimer(hWnd, 0x1234);
			ShowWindow(hWnd, SW_SHOW);
			if (IsIconic(hWnd) != 0) {
				ShowWindow(hWnd, SW_RESTORE);
			}
			SetForegroundWindow(hWnd);
			break;
		case WM_RBUTTONDOWN:
			SetForegroundWindow(hWnd);
			GetCursorPos((LPPOINT)&point);
			TrackPopupMenu(GetSubMenu(LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU1)), 0), TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_LEFTBUTTON,
				point.x, point.y, 0, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
			break;
		}
		break;
	case WM_DESTROY:
		tnd.cbSize = sizeof(NOTIFYICONDATA);
		tnd.hWnd = hWnd;
		tnd.uID = TRAY_ID;
		tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		tnd.uCallbackMessage = MYWM_NOTIFYICON;
		tnd.hIcon = NULL;
		tnd.szTip[0] = L'\0';
		Shell_NotifyIcon(NIM_DELETE, &tnd);
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

	ULONG_PTR gdiplusToken;
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

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
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}
