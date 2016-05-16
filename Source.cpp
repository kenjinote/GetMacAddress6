#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include <ntddndis.h>

TCHAR szClassName[] = TEXT("Window");

void GetMacAddress(HWND hList)
{
	// GUID は コマンドプロンプトで「wmic path Win32_NetworkAdapterConfiguration get * /format:list」コマンドで取得できる。
	HANDLE hDevice = CreateFile(
		TEXT("\\\\.\\{7D835572-013F-408D-89CE-82340CFB9572}"),
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return;
	}		
	unsigned int code = OID_802_3_CURRENT_ADDRESS; // 元のアドレスを取得するには OID_802_3_PERMANENT_ADDRESS を指定する
	BYTE macAddress[6];
	DWORD bytesReturned;
	DeviceIoControl(
		hDevice,
		IOCTL_NDIS_QUERY_GLOBAL_STATS,
		&code,
		sizeof(code),
		macAddress,
		sizeof(macAddress),
		&bytesReturned,
		NULL
	);
	CloseHandle(hDevice);
	TCHAR szMacAddress[64];
	wsprintf(szMacAddress, TEXT("%02X-%02X-%02X-%02X-%02X-%02X"),
		macAddress[0],
		macAddress[1],
		macAddress[2],
		macAddress[3],
		macAddress[4],
		macAddress[5]);
	SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)szMacAddress);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hButton;
	static HWND hList;
	switch (msg)
	{
	case WM_CREATE:
		hList = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), 0, WS_VISIBLE | WS_CHILD | WS_VSCROLL | LBS_NOINTEGRALHEIGHT, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("Macアドレスを取得"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		PostMessage(hWnd, WM_COMMAND, IDOK, 0);
		break;
	case WM_SIZE:
		MoveWindow(hList, 10, 10, 256, 128, TRUE);
		MoveWindow(hButton, 10, 148, 256, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			SendMessage(hList, LB_RESETCONTENT, 0, 0);
			GetMacAddress(hList);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("DeviceIoControl 関数を使用してMacアドレスを取得する"),
		WS_OVERLAPPEDWINDOW,
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
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
