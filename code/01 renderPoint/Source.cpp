#include <Windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN
bool running = true;

void* buffer_memory;
int buffer_width, buffer_height;
BITMAPINFO buffer_bitmap_info;


//-----------------------------------------------

// Entry point
int CALLBACK WinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPSTR lpCmdLine,
					 _In_ int nCmdShow) {
	WCHAR cls_Name[] = L"Win32 Class";
	WNDCLASS wc = { };

	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = cls_Name;
	wc.hInstance = hInstance;

	RegisterClass(&wc);

	// create
	HWND hwnd = CreateWindow(
		cls_Name,			//類別名稱，和剛剛一致
		L"Win32 test",		//標題文字
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, //視窗外觀樣式
		100,				//相對於parent的x座標
		200,				//相對於parent的y座標
		800,				//視窗寬度
		600,				//視窗高度
		NULL,				//parent window，沒的話NULL即可
		NULL,				//menu
		hInstance,			//當前程式的追蹤
		NULL);				//附加資料
	HDC hdc = GetDC(hwnd);

	if (hwnd == NULL) //判斷是否成功Create
		return 0;

	// 顯示視窗 dxstyle =| WS_VISIBLE 就不用這行
	//ShowWindow(hwnd, SW_SHOW);

	// 事件訊息loop
	while (running) {
		// Input
		MSG msg;
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Update
		unsigned int* pixel = (unsigned int*)buffer_memory;
		for (size_t y = 0; y < buffer_height; y++)
		{
			for (size_t x = 0; x < buffer_width; x++)
			{
				*pixel++ = 0x000000;
			}
		}
		pixel = (unsigned int*)buffer_memory;
		int y = 10;
		for (size_t x = 0; x < 100; x++)
		{
			*(pixel + buffer_width * (buffer_height - y) + x) = 0xff0000;
		}

		// Render
		StretchDIBits(
			hdc,
			0, 0,
			buffer_width, buffer_height,
			0, 0,
			buffer_width, buffer_height,
			buffer_memory,
			&buffer_bitmap_info, DIB_RGB_COLORS, SRCCOPY);
	}
	return 0;
}

// callback事件處裡 function pointer
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_CLOSE:
		case WM_DESTROY: {
			running = false;
			break;
		}
					   // when window change
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);// 不包含標題列的區域
			buffer_width = rect.right - rect.left;
			buffer_height = rect.bottom - rect.top;

			int buffer_size = buffer_width * buffer_height * sizeof(unsigned int);

			if (buffer_memory) VirtualFree(buffer_memory, 0, MEM_RELEASE);
			buffer_memory = VirtualAlloc(0, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

			buffer_bitmap_info.bmiHeader.biSize = sizeof(buffer_bitmap_info.bmiHeader);
			buffer_bitmap_info.bmiHeader.biWidth = buffer_width;
			buffer_bitmap_info.bmiHeader.biHeight = buffer_height;
			buffer_bitmap_info.bmiHeader.biPlanes = 1;
			buffer_bitmap_info.bmiHeader.biBitCount = 32;
			buffer_bitmap_info.bmiHeader.biCompression = BI_RGB;
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}