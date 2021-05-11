#include <Windows.h>
#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <wchar.h>

using namespace std::chrono;

typedef unsigned int u32;
typedef struct Vec2 { float x, y; }vec2;
typedef struct Vec3 { float x, y, z; }vec3;

void drawPoint(int x, int y, u32 c);
void drawLine(int x1, int y1, int x2, int y2, u32 c);
void DDA(int x, int y, int w, int h, u32 c);
void bresenham2(int x1, int y1, int x2, int y2, u32 c);
void drawRect(int x, int y, int w, int h, u32 c);
void drawTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c);
void fillTopFlatTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c);
void fillBottomFlatTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c);
void fillTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c);

void fpsCounting();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN
int __cdecl printf2(const char* format, ...);

//-----------------------------------------------
// global variable
bool running = true;

void* buffer_memory;
int buffer_width, buffer_height;
BITMAPINFO buffer_bitmap_info;

//-----------------------------------------------
// just test

u32 backcolor = 0xFFFFFF;//0xAARRGGBB;

vec2 rect{ 0,0 }, rectS{ 50,50 }, rectV{ 0.25,0.5 };
vec2 v1{ 200,300 }, v2{ 250,400 }, v3{ 100,350 };

uint64_t timeSinceEpochMillisec() {
	using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
int t1 = timeSinceEpochMillisec(), dt = 0, totalT = 0;;
int fps = 0, frames = 0;
//-----------------------------------------------
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
void update() {
	//printf2("x:%f, y:%f\n", rect.x, rect.y);

	rect.x += rectV.x;
	rect.y += rectV.y;
	if (rect.x <= 0 || rect.x + rectS.x >= buffer_width - 1) rectV.x *= -1;
	if (rect.y <= 0 || rect.y + rectS.y >= buffer_height - 1) rectV.y *= -1;
	//printf2("x:%f, y:%f\n", rect.x, rect.y);

}

void render() {
	// clear
	clear(backcolor);

	int len = 100;
	int cx1 = 110, cy1 = 110;
	const float PI = 3.14159265358979f;
	//drawLine(0, 10, 100, 100, 0);
	for (float i = 0; i < PI * 2; i += PI / 8)
	{
		DDA(cx1, cy1, cx1 + len * cos(i), cy1 + len * sin(i), 0x000000);
		drawLine(cx1 + 250, cy1, cx1 + 250 + len * cos(i), cy1 + len * sin(i), 0x0000FF);
		bresenham2(cx1 + 500, cy1, cx1 + 500 + len * cos(i), cy1 + len * sin(i), 0x0000FF);
	}

	fillTriangle(v1, v2, v3, 0xFF7700);
	//drawTriangle(v1, v2, v3, 0);
	//DDA(0, 30, buffer_width, 30, 0x000000);
	//DDA(100, 100, 150, 80, 0x000000);

	//drawLine(100, 100, 200, 300, 0x000000);
	drawRect(rect.x, rect.y, rectS.x, rectS.y, 0x0000FF);
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

	wchar_t strBuffer[100];
	while (running) {
		fpsCounting();
		swprintf(strBuffer, 100, L"win32 test FPS:%4d dt:%2dms", fps, dt);
		SetWindowText(hwnd, strBuffer);
		// Input
		MSG msg;
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Update
		update();
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

void fpsCounting() {
	dt = timeSinceEpochMillisec() - t1;
	totalT += dt;
	frames++;
	//printf2("%d\n", totalT);
	if (totalT >= 1000) {
		totalT -= 1000;
		//printf2("FPS:%d\n", fpsCount);
		fps = frames;
		frames = 0;
	}
	t1 = timeSinceEpochMillisec();
}


void drawPoint(int x, int y, u32 c) {
	if (x < 0 || x >= buffer_width || y < 0 || y >= buffer_height)return;
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
		if (dx >= dy) { // m <= 1
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
		else {// m > 1
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

void DDA(int x1, int y1, int x2, int y2, u32 c) {
	int dx = x2 - x1,
		dy = y2 - y1;

	int step = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	float xi = (float)dx / step, yi = (float)dy / step;

	float x = x1, y = y1;

	drawPoint(x1, y1, c);
	drawPoint(x2, y2, c);

	for (int i = 0; i < step; i++) {
		x += xi;
		y += yi;
		drawPoint(round(x), round(y), c);
	}
}
void bresenham2(int x1, int y1, int x2, int y2, u32 c) {
	int steep = (abs(y2 - y1) > abs(x2 - x1)) ? 1 : 0;
	if (steep) {
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	if (x1 > x2) {
		std::swap(x1, x2);
		std::swap(y1, y2);
	}

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int p = 2 * dy - dx;
	int const1 = 2 * dy;
	int const2 = 2 * (dy - dx);

	int inc = (y1 < y2) ? 1 : -1;

	int x = x1, y = y1;
	while (x <= x2) {
		if (steep) drawPoint(y, x, c);
		else drawPoint(x, y, c);

		x++;

		if (p < 0) p += const1;
		else {
			y += inc;
			p += const2;
		}
	}
}
void drawRect(int x, int y, int w, int h, u32 c) {
	drawLine(x, y, x + w, y, c);
	drawLine(x, y, x, y + h, c);
	drawLine(x + w, y, x + w, y + h, c);
	drawLine(x, y + h, x + w, y + h, c);
}
void drawTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c) {
	drawLine(v1.x, v1.y, v2.x, v2.y, c);
	drawLine(v2.x, v2.y, v3.x, v3.y, c);
	drawLine(v3.x, v3.y, v1.x, v1.y, c);
}
void fillTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c) {
	/* at first sort the three vertices by y-coordinate ascending so v1 is the topmost vertice */
	//sortVerticesAscendingByY();
	if (v1.y > v2.y)std::swap(v1, v2);
	if (v2.y > v3.y)std::swap(v2, v3);
	if (v1.y > v2.y)std::swap(v1, v2);

	/* here we know that v1.y <= v2.y <= v3.y */
	/* check for trivial case of bottom-flat triangle */
	if (v2.y == v3.y)
	{
		fillBottomFlatTriangle(v1, v2, v3, c);
	}
	/* check for trivial case of top-flat triangle */
	else if (v1.y == v2.y)
	{
		fillTopFlatTriangle(v1, v2, v3, c);
	}
	else
	{
		/* general case - split the triangle in a topflat and bottom-flat one */
		vec2 v4{ (int)(v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x)), v2.y };
		fillBottomFlatTriangle(v1, v2, v4, c);
		fillTopFlatTriangle(v2, v4, v3, c);
	}
}

void fillBottomFlatTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c) {
	float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
	float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);

	float curx1 = v1.x;
	float curx2 = v1.x;

	for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++)
	{
		drawLine((int)curx1, scanlineY, (int)curx2, scanlineY, c);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}
void fillTopFlatTriangle(vec2 v1, vec2 v2, vec2 v3, u32 c) {
	float invslope1 = (v3.x - v1.x) / (v3.y - v1.y);
	float invslope2 = (v3.x - v2.x) / (v3.y - v2.y);

	float curx1 = v3.x;
	float curx2 = v3.x;

	for (int scanlineY = v3.y; scanlineY > v1.y; scanlineY--)
	{
		drawLine((int)curx1, scanlineY, (int)curx2, scanlineY, c);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}

int __cdecl printf2(const char* format, ...)
{
	char str[1024];

	va_list argptr;
	va_start(argptr, format);
	int ret = vsnprintf(str, sizeof(str), format, argptr);
	va_end(argptr);

	OutputDebugStringA(str);

	return ret;
}