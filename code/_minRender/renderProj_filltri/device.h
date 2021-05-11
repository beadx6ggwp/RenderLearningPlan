#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <assert.h>
#include <algorithm> // std::swap

#include <windows.h>
#include <tchar.h>

#include "dataStruct.h"

using namespace std;

typedef unsigned int IUINT32;


//=====================================================================
// 顯示
//=====================================================================
typedef struct {
	int width;                  // 寬度
	int height;                 // 高度
	IUINT32** framebuffer;      // pixelbuffer：framebuffer[y] 代表第 y行
	float** zbuffer;            // 深度緩衝：zbuffer[y] 為第 y行指針
	IUINT32** texture;          // 紋理：同樣是每行索引
	int tex_width;              // 紋理寬度
	int tex_height;             // 紋理高度
	float max_u;                // 紋理最大寬度：tex_width - 1
	float max_v;                // 紋理最大高度：tex_height - 1
	int render_state;           // 渲染狀態
	IUINT32 background;         // 背景顏色
}	device_t;

#define RENDER_STATE_WIREFRAME      1		// 渲染線框
#define RENDER_STATE_TEXTURE        2		// 渲染紋理
#define RENDER_STATE_COLOR          4		// 渲染顏色

// 設備初始化，fb為外部幀緩衝，非 NULL 將引用外部幀緩存（每行 4字節對齊）
void device_init(device_t* device, int width, int height, void* fb) {
	int need = sizeof(void*) * (height * 2 + 1024) + width * height * 8;
	char* ptr = (char*)malloc(need + 64);
	char* framebuf, * zbuf;
	int j;
	assert(ptr);
	device->framebuffer = (IUINT32**)ptr;
	device->zbuffer = (float**)(ptr + sizeof(void*) * height);
	ptr += sizeof(void*) * height * 2;
	device->texture = (IUINT32**)ptr;
	ptr += sizeof(void*) * 1024;
	framebuf = (char*)ptr;
	zbuf = (char*)ptr + width * height * 4;
	ptr += width * height * 8;
	if (fb != NULL) framebuf = (char*)fb;
	for (j = 0; j < height; j++) {
		device->framebuffer[j] = (IUINT32*)(framebuf + width * 4 * j);
		device->zbuffer[j] = (float*)(zbuf + width * 4 * j);
	}
	device->texture[0] = (IUINT32*)ptr;
	device->texture[1] = (IUINT32*)(ptr + 16);
	memset(device->texture[0], 0, 64);
	device->tex_width = 2;
	device->tex_height = 2;
	device->max_u = 1.0f;
	device->max_v = 1.0f;
	device->width = width;
	device->height = height;
	device->background = 0xFFFFFF;
	device->render_state = RENDER_STATE_WIREFRAME;
}

// 刪除設備
void device_destroy(device_t* device) {
	if (device->framebuffer)
		free(device->framebuffer);
	device->framebuffer = NULL;
	device->zbuffer = NULL;
	device->texture = NULL;
}

// 清空 framebuffer 和 zbuffer
void device_clear(device_t* device, int mode) {
	int y, x, height = device->height;
	for (y = 0; y < device->height; y++) {
		IUINT32* dst = device->framebuffer[y];
		IUINT32 cc = (height - 1 - y) * 230 / (height - 1);
		cc = (cc << 16) | (cc << 8) | cc;
		if (mode == 0) cc = device->background;
		for (x = device->width; x > 0; dst++, x--) dst[0] = cc;
	}
	for (y = 0; y < device->height; y++) {
		float* dst = device->zbuffer[y];
		for (x = device->width; x > 0; dst++, x--) dst[0] = 0.0f;
	}
}

// 畫點
void device_pixel(device_t* device, int x, int y, IUINT32 color) {
	if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height) {
		device->framebuffer[y][x] = color;
	}
}

// 繪制線段
void drawLine(device_t* device, int x1, int y1, int x2, int y2, IUINT32 c) {
	int x, y, rem = 0;
	if (x1 == x2 && y1 == y2) {
		device_pixel(device, x1, y1, c);
	}
	else if (x1 == x2) {
		int inc = (y1 <= y2) ? 1 : -1;
		for (y = y1; y != y2; y += inc) device_pixel(device, x1, y, c);
		device_pixel(device, x2, y2, c);
	}
	else if (y1 == y2) {
		int inc = (x1 <= x2) ? 1 : -1;
		for (x = x1; x != x2; x += inc) device_pixel(device, x, y1, c);
		device_pixel(device, x2, y2, c);
	}
	else {
		int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
		int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
		if (dx >= dy) {
			if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; x <= x2; x++) {
				device_pixel(device, x, y, c);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y2 >= y1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
		else {
			if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
			for (x = x1, y = y1; y <= y2; y++) {
				device_pixel(device, x, y, c);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x2 >= x1) ? 1 : -1;
					device_pixel(device, x, y, c);
				}
			}
			device_pixel(device, x2, y2, c);
		}
	}
}



void DDA(device_t* device, int x1, int y1, int x2, int y2, u32 c) {
	int dx = x2 - x1,
		dy = y2 - y1;

	int step = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	float xi = (float)dx / step, yi = (float)dy / step;

	float x = x1, y = y1;

	device_pixel(device, x1, y1, c);
	device_pixel(device, x2, y2, c);

	for (int i = 0; i < step; i++) {
		x += xi;
		y += yi;
		device_pixel(device, round(x), round(y), c);
	}
}
void bresenham2(device_t* device, int x1, int y1, int x2, int y2, u32 c) {
	int steep = (abs(y2 - y1) > abs(x2 - x1)) ? 1 : 0;
	if (steep) {
		// m>1
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
		if (steep) device_pixel(device, y, x, c);
		else device_pixel(device, x, y, c);

		x++;

		if (p < 0) p += const1;
		else {
			y += inc;
			p += const2;
		}
	}
}
void drawRect(device_t* device, int x, int y, int w, int h, u32 c) {
	drawLine(device, x, y, x + w, y, c);
	drawLine(device, x, y, x, y + h, c);
	drawLine(device, x + w, y, x + w, y + h, c);
	drawLine(device, x, y + h, x + w, y + h, c);
}
void drawTriangle(device_t* device, vec3 v1, vec3 v2, vec3 v3, u32 c) {
	drawLine(device, v1.x, v1.y, v2.x, v2.y, c);
	drawLine(device, v2.x, v2.y, v3.x, v3.y, c);
	drawLine(device, v3.x, v3.y, v1.x, v1.y, c);
}

void fillBottomFlatTriangle(device_t* device, vec3 v1, vec3 v2, vec3 v3, u32 c) {
	float invslope1 = (v2.x - v1.x) / (v2.y - v1.y);
	float invslope2 = (v3.x - v1.x) / (v3.y - v1.y);

	float curx1 = v1.x;
	float curx2 = v1.x;

	for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++)
	{
		drawLine(device, (int)curx1, scanlineY, (int)curx2, scanlineY, c);
		curx1 += invslope1;
		curx2 += invslope2;
	}
}
void fillTopFlatTriangle(device_t* device, vec3 v1, vec3 v2, vec3 v3, u32 c) {
	float invslope1 = (v3.x - v1.x) / (v3.y - v1.y);
	float invslope2 = (v3.x - v2.x) / (v3.y - v2.y);

	float curx1 = v3.x;
	float curx2 = v3.x;

	for (int scanlineY = v3.y; scanlineY > v1.y; scanlineY--)
	{
		drawLine(device, (int)curx1, scanlineY, (int)curx2, scanlineY, c);
		curx1 -= invslope1;
		curx2 -= invslope2;
	}
}
void fillTriangle(device_t* device, vec3 v1, vec3 v2, vec3 v3, u32 c) {
	if (v1.y > v2.y)std::swap(v1, v2);
	if (v2.y > v3.y)std::swap(v2, v3);
	if (v1.y > v2.y)std::swap(v1, v2);

	if (v2.y == v3.y)
	{
		fillBottomFlatTriangle(device, v1, v2, v3, c);
	}
	else if (v1.y == v2.y)
	{
		fillTopFlatTriangle(device, v1, v2, v3, c);
	}
	else
	{
		vec3 v4{ (v1.x + ((v2.y - v1.y) / (v3.y - v1.y)) * (v3.x - v1.x)), v2.y };
		fillBottomFlatTriangle(device, v1, v2, v4, c);
		fillTopFlatTriangle(device, v2, v4, v3, c);
	}
}

//=====================================================================
// Win32 繪製：為 device 提供一個 DibSection 的 FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];
static HWND screen_handle = NULL;		// 主窗口 HWND
static HDC screen_dc = NULL;			// 配套的 HDC
static HBITMAP screen_hb = NULL;		// DIB
static HBITMAP screen_ob = NULL;		// old_bitmap
unsigned char* screen_fb = NULL;		// frame buffer
long screen_pitch = 0;

int screen_init(int w, int h, const TCHAR* title);
int screen_close(void);
void screen_dispatch(void);
void screen_update(void);

// win32 event handler
static LRESULT screen_events(HWND, UINT, WPARAM, LPARAM);

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

// 初始化窗口並設置標題
int screen_init(int w, int h, const TCHAR* title) {
	WNDCLASS wc = { CS_BYTEALIGNCLIENT, (WNDPROC)screen_events, 0, 0, 0,
		NULL, NULL, NULL, NULL, _T("SCREEN3.1415926") };
	BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), w, -h, 1, 32, BI_RGB,
		w * h * 4, 0, 0, 0, 0 } };
	RECT rect = { 0, 0, w, h };
	int wx, wy, sx, sy;
	LPVOID ptr;
	HDC hDC;

	screen_close();

	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance = GetModuleHandle(NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	if (!RegisterClass(&wc)) return -1;

	screen_handle = CreateWindow(_T("SCREEN3.1415926"), title,
								 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
								 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);
	if (screen_handle == NULL) return -2;

	screen_exit = 0;
	hDC = GetDC(screen_handle);
	screen_dc = CreateCompatibleDC(hDC);
	ReleaseDC(screen_handle, hDC);

	screen_hb = CreateDIBSection(screen_dc, &bi, DIB_RGB_COLORS, &ptr, 0, 0);
	if (screen_hb == NULL) return -3;

	screen_ob = (HBITMAP)SelectObject(screen_dc, screen_hb);
	screen_fb = (unsigned char*)ptr;
	screen_w = w;
	screen_h = h;
	screen_pitch = w * 4;

	AdjustWindowRect(&rect, GetWindowLong(screen_handle, GWL_STYLE), 0);
	wx = rect.right - rect.left;
	wy = rect.bottom - rect.top;
	sx = (GetSystemMetrics(SM_CXSCREEN) - wx) / 2;
	sy = (GetSystemMetrics(SM_CYSCREEN) - wy) / 2;
	if (sy < 0) sy = 0;
	SetWindowPos(screen_handle, NULL, sx, sy, wx, wy, (SWP_NOCOPYBITS | SWP_NOZORDER | SWP_SHOWWINDOW));
	SetForegroundWindow(screen_handle);

	ShowWindow(screen_handle, SW_NORMAL);
	screen_dispatch();

	memset(screen_keys, 0, sizeof(int) * 512);
	memset(screen_fb, 0, w * h * 4);

	return 0;
}

int screen_close(void) {
	if (screen_dc) {
		if (screen_ob) {
			SelectObject(screen_dc, screen_ob);
			screen_ob = NULL;
		}
		DeleteDC(screen_dc);
		screen_dc = NULL;
	}
	if (screen_hb) {
		DeleteObject(screen_hb);
		screen_hb = NULL;
	}
	if (screen_handle) {
		CloseWindow(screen_handle);
		screen_handle = NULL;
	}
	return 0;
}

static LRESULT screen_events(HWND hWnd, UINT msg,
							 WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_CLOSE: screen_exit = 1; break;
		case WM_KEYDOWN: screen_keys[wParam & 511] = 1; break;
		case WM_KEYUP: screen_keys[wParam & 511] = 0; break;
		default: return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

void screen_dispatch(void) {
	MSG msg;
	while (1) {
		if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) break;
		if (!GetMessage(&msg, NULL, 0, 0)) break;
		DispatchMessage(&msg);
	}
}

void screen_update(void) {
	HDC hDC = GetDC(screen_handle);
	BitBlt(hDC, 0, 0, screen_w, screen_h, screen_dc, 0, 0, SRCCOPY);
	ReleaseDC(screen_handle, hDC);
	screen_dispatch();
}