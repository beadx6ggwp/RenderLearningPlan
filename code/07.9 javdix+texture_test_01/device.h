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
// ���
//=====================================================================
typedef struct {
	int width;                  // �e��
	int height;                 // ����
	IUINT32** framebuffer;      // pixelbuffer�Gframebuffer[y] �N��� y��
	float** zbuffer;            // �`�׽w�ġGzbuffer[y] ���� y����w
	IUINT32** texture;          // ���z�G�P�ˬO�C�����
	int tex_width;              // ���z�e��
	int tex_height;             // ���z����
	float max_u;                // ���z�̤j�e�סGtex_width - 1
	float max_v;                // ���z�̤j���סGtex_height - 1
	int render_state;           // ��V���A
	IUINT32 background;         // �I���C��
}	device_t;

#define RENDER_STATE_WIREFRAME      1		// ��V�u��
#define RENDER_STATE_TEXTURE        2		// ��V���z
#define RENDER_STATE_COLOR          4		// ��V�C��

// �]�ƪ�l�ơAfb���~���V�w�ġA�D NULL �N�ޥΥ~���V�w�s�]�C�� 4�r�`����^
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

// �R���]��
void device_destroy(device_t* device) {
	if (device->framebuffer)
		free(device->framebuffer);
	device->framebuffer = NULL;
	device->zbuffer = NULL;
	device->texture = NULL;
}

// �M�� framebuffer �M zbuffer
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
		for (x = device->width; x > 0; dst++, x--) dst[0] = -(std::numeric_limits<float>::max)();
	}
}

// �e�I
void device_pixel(device_t* device, int x, int y, IUINT32 color) {
	if (((IUINT32)x) < (IUINT32)device->width && ((IUINT32)y) < (IUINT32)device->height) {
		device->framebuffer[y][x] = color;
	}
}

// ø��u�q
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

// https://www.avrfreaks.net/sites/default/files/triangles.c
void fillTriangle2(device_t* device, vec3 v1, vec3 v2, vec3 v3, u32 c)
{
	auto SWAP = [](int& x, int& y) { int t = x; x = y; y = t; };
	auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) device_pixel(device, i, ny, c); };
	int x1 = v1.x, y1 = v1.y;
	int x2 = v2.x, y2 = v2.y;
	int x3 = v3.x, y3 = v3.y;
	int t1x, t2x, y, minx, maxx, t1xp, t2xp;
	bool changed1 = false;
	bool changed2 = false;
	int signx1, signx2, dx1, dy1, dx2, dy2;
	int e1, e2;
	// Sort vertices
	if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
	if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
	if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

	t1x = t2x = x1; y = y1;   // Starting points
	dx1 = (int)(x2 - x1); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (int)(y2 - y1);

	dx2 = (int)(x3 - x1); if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
	else signx2 = 1;
	dy2 = (int)(y3 - y1);

	if (dy1 > dx1) {   // swap values
		SWAP(dx1, dy1);
		changed1 = true;
	}
	if (dy2 > dx2) {   // swap values
		SWAP(dy2, dx2);
		changed2 = true;
	}

	e2 = (int)(dx2 >> 1);
	// Flat top, just process the second half
	if (y1 == y2) goto next;
	e1 = (int)(dx1 >> 1);

	for (int i = 0; i < dx1;) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			i++;
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) t1xp = signx1;//t1x += signx1;
				else          goto next1;
			}
			if (changed1) break;
			else t1x += signx1;
		}
		// Move line
	next1:
		// process second line until y value is about to change
		while (1) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;//t2x += signx2;
				else          goto next2;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next2:
		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);    // Draw line from min to max points found on the y
									 // Now increase y
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y == y2) break;

	}
next:
	// Second half
	dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (int)(y3 - y2);
	t1x = x2;

	if (dy1 > dx1) {   // swap values
		SWAP(dy1, dx1);
		changed1 = true;
	}
	else changed1 = false;

	e1 = (int)(dx1 >> 1);

	for (int i = 0; i <= dx1; i++) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) { t1xp = signx1; break; }//t1x += signx1;
				else          goto next3;
			}
			if (changed1) break;
			else   	   	  t1x += signx1;
			if (i < dx1) i++;
		}
	next3:
		// process second line until y value is about to change
		while (t2x != x3) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;
				else          goto next4;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next4:

		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y > y3) return;
	}
}
int insideTriangle(triangle t, float x, float y) {
	vec3 p = { x, y, 0, 1 };
	vec3 ab = Vector_Sub(t.p[1], t.p[0]);
	vec3 bc = Vector_Sub(t.p[2], t.p[1]);
	vec3 ca = Vector_Sub(t.p[0], t.p[2]);

	vec3 ap = Vector_Sub(p, t.p[0]);
	vec3 bp = Vector_Sub(p, t.p[1]);
	vec3 cp = Vector_Sub(p, t.p[2]);
	float f1 = Vector_CrossProduct(ab, ap).z;
	float f2 = Vector_CrossProduct(bc, bp).z;
	float f3 = Vector_CrossProduct(ca, cp).z;
	return (f1 > 0) && (f2 > 0) && (f3 > 0);
}
void simpleMsaaTriangle(device_t* device, vec3 v1, vec3 v2, vec3 v3, u32 c) {
	//msaa x4
	float min_x = min(v1.x, min(v2.x, v3.x));
	float max_x = max(v1.x, max(v2.x, v3.x));
	float min_y = min(v1.y, min(v2.y, v3.y));
	float max_y = max(v1.y, max(v2.y, v3.y));

	min_x = (int)std::floor(min_x);
	max_x = (int)std::ceil(max_x);
	min_y = (int)std::floor(min_y);
	max_y = (int)std::ceil(max_y);

	/*int postotal = 4;
	vec3 pos[] = {
		{0.25,0.25},
		{0.75,0.25},
		{0.25,0.75},
		{0.75,0.75},
	};*/

	int postotal = 16;
	vec3 pos[] = {
		{0.125,0.125},
		{0.125,0.375},
		{0.125,0.625},
		{0.125,0.875},

		{0.375,0.125},
		{0.375,0.375},
		{0.375,0.625},
		{0.375,0.875},

		{0.625,0.125},
		{0.625,0.375},
		{0.625,0.625},
		{0.625,0.875},

		{0.875,0.125},
		{0.875,0.375},
		{0.875,0.625},
		{0.875,0.875},
	};

	for (int x = min_x; x <= max_x; x++) {
		for (int y = min_y; y <= max_y; y++) {
			int count = 0;
			for (int i = 0; i < postotal; i++) {
				triangle t = { v1,v2,v3 };
				if (insideTriangle(t, (float)x + pos[i].x, (float)y + pos[i].y)) {
					count++;
				}
			}
			if (count != 0) {
				int r = (c & 0xFF0000) >> 16;
				int g = (c & 0x00FF00) >> 8;
				int b = (c & 0x0000FF);

				int newr = (int)(r * count / (float)postotal) << 16;
				int newg = (int)(g * count / (float)postotal) << 8;
				int newb = (int)(b * count / (float)postotal);
				u32 newC = newr | newg | newb;
				device_pixel(device, x, y, newC);
			}
		}
	}
}

void drawDebugTriangleLine(device_t* device, vec3 v1, vec3 v2, vec3 v3) {
	// ����
	vec3 center = { (v1.x + v2.x + v3.x) / 3 ,(v1.y + v2.y + v3.y) / 3 };
	/*
	   a
	   / \
	  /cnt\
	b/_____\c

	*/
	vec3 cntA = Vector_Sub(v1, center);
	vec3 cntB = Vector_Sub(v2, center);
	vec3 cntC = Vector_Sub(v3, center);

	// new length
	vec3 nlA = Vector_Mul(cntA, 0.95);
	vec3 nlB = Vector_Mul(cntB, 0.95);
	vec3 nlC = Vector_Mul(cntC, 0.95);
	// new vetex
	vec3 nA = Vector_Add(center, nlA);
	vec3 nB = Vector_Add(center, nlB);
	vec3 nC = Vector_Add(center, nlC);

	drawLine(device, nA.x, nA.y, nB.x, nB.y, 0xff5555);
	drawLine(device, nB.x, nB.y, nC.x, nC.y, 0x55ff55);
	drawLine(device, nC.x, nC.y, nA.x, nA.y, 0x5555ff);
}


//------------------------------------------------------------------
// render device

//=====================================================================
// Win32 ø�s�G�� device ���Ѥ@�� DibSection �� FB
//=====================================================================
int screen_w, screen_h, screen_exit = 0;
int screen_mx = 0, screen_my = 0, screen_mb = 0;
int screen_keys[512];
static HWND screen_handle = NULL;		// �D���f HWND
static HDC screen_dc = NULL;			// �t�M�� HDC
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

// ��l�Ƶ��f�ó]�m���D
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

	// �Nscreen_fb �P screen_hb�����Vptr�H���@�P�j�w�Nscreen_fb�thb
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