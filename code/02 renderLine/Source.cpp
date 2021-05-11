#include <Windows.h>

typedef unsigned int u32;
void drawPoint(int x, int y, u32 c);
void drawLine(int x1, int y1, int x2, int y2, u32 c);
void drawRect(int x, int y, int w, int h, u32 c);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN

//-----------------------------------------------
// global variable
bool running = true;

void* buffer_memory;
int buffer_width, buffer_height;
BITMAPINFO buffer_bitmap_info;


//-----------------------------------------------
// just test

u32 backcolor = 0x999999;//0xAARRGGBB;
typedef struct Vec { 
	float x, y; 
}vec;

float x = 0, y = 200, w = 70, h = 70, vx = 0.25, vy = 0.5;


//-----------------------------------------------
void update() {
	x += vx;
	y += vy;
	if (x <= 0 || x + w >= buffer_width - 1) vx *= -1;
	if (y <= 0 || y + h >= buffer_height - 1) vy *= -1;
}

void render() {
	drawLine(100, 100, 200, 300, 0xFF0000);
	drawRect(x, y, w, h, 0x000000);
}

void clear(u32 c) {
	u32* pixel = (u32*)buffer_memory;
	for (size_t y = 0; y < buffer_height; y++)
	{
		for (size_t x = 0; x < buffer_width; x++)
		{
			*pixel++ = c;
		}
	}
}
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
		update();
		// clear
		clear(backcolor);
		render();

		// Render on app
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

			int buffer_size = buffer_width * buffer_height * sizeof(u32);

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




void drawPoint(int x, int y, u32 c) {
	u32* pixel = (u32*)buffer_memory;
	*(pixel + buffer_width * (buffer_height - 1 - y) + x) = c;
}

void drawLine(int x1, int y1, int x2, int y2, u32 c) {
	// using DDA
	int x, y, rem = 0;
	if (x1 == x2 && y1 == y2) {
		drawPoint(x1, y1, c);
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) drawPoint(x1, y, c);
		drawPoint(x2, y2, c);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) drawPoint(x, y1, c);
		drawPoint(x2, y2, c);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		if (dx >= dy) {
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++) {
				drawPoint(x, y, c);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y2 >= y1) ? 1 : -1;
					drawPoint(x, y, c);
				}
			}
			drawPoint(x2, y2, c);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++) {
				drawPoint(x, y, c);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x2 >= x1) ? 1 : -1;
					drawPoint(x, y, c);
				}
			}
			drawPoint(x2, y2, c);
		}
	}
}

void drawRect(int x, int y, int w, int h, u32 c) {
	drawLine(x, y, x + w, y, c);
	drawLine(x, y, x, y + h, c);
	drawLine(x + w, y, x + w, y + h, c);
	drawLine(x, y + h, x + w, y + h, c);
}