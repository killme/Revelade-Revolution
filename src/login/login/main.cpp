#include <windows.h>

// set some stuff
#define WS_EX_LAYERED   0x00080000
#define LWA_ALPHA       0x00000002
#define MYCLASSNAME ("__transparentwnd__")

// set the SLWA type
typedef BOOL(WINAPI *SLWA)(HWND, COLORREF, BYTE, DWORD); 

// Main window message loop
LRESULT CALLBACK WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
        // When the window is destroyed
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}
#define IDI_ICON1 1;

// Create the window
int WINAPI Create_Window_Transparent(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
							   LPTSTR lpCommandLine, int nShowCmd, LPCTSTR title)
{
	nShowCmd = 0;
	


        // This stuff gets some other stuff for the transparency
	SLWA pSetLayeredWindowAttributes = NULL; 
	HINSTANCE hmodUSER32 = LoadLibrary("USER32.DLL"); 
	pSetLayeredWindowAttributes =(SLWA)GetProcAddress(hmodUSER32,"SetLayeredWindowAttributes");
	int posX, posY;
	DEVMODE dmScreenSettings;

        // Create the window's class
	MSG msg;
	WNDCLASS wc = {0};
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.lpszClassName = MYCLASSNAME;
	wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	wc.hIcon = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WinProc;
	#include "res.h"
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MyIcon));

        // Register the windows class
	RegisterClass(&wc);

        // Create the actual window
	HWND hWnd = CreateWindowEx(WS_EX_LAYERED, MYCLASSNAME, title, WS_POPUP, 0, 0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),

		NULL, NULL, hInstance, NULL);

        // Make it transparent
	pSetLayeredWindowAttributes(hWnd, 0, 100, LWA_ALPHA);

        // Show the window
	//SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
	//ShowWindow(hWnd, nShowCmd);
	//UpdateWindow(hWnd);
	ShellExecute(GetDesktopWindow(), "open", "bin\\R+R_tes.exe", NULL, NULL, SW_SHOWNORMAL);
	//system("www.google.com");
	return -1;
        // Simple Message loop
	BOOL bRet = false;
	/*while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0) { 
		if (bRet == -1) break;
		else {
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}*/
	//}
	return -1;
}

// Main function, called on execution
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCommandLine, int nShowCmd)
{
        // Create the window using the function above
	Create_Window_Transparent(hInstance, hPrevInstance, lpCommandLine, nShowCmd, "My Transparent Window!");
}