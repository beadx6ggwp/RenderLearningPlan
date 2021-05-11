#include <Windows.h>
#include <algorithm> // std::swap
#include <cmath>
#include <chrono> // timer
#include <wchar.h>// swprintf
#include <vector>

using namespace std;
using namespace std::chrono;
//---------------------------------
// data type
typedef unsigned int u32;
typedef struct _Vec2 { float x, y; } vec2;
typedef struct _Vec3 { float x, y, z; } vec3;

typedef struct _Triangle { vec3 p[3]; } triangle;
typedef struct _Mesh { vector<triangle> tris; } mesh;

typedef struct MATRIX4X4_TYP {
	union {
		float M[4][4];

		struct {
			float M00, M01, M02, M03;
			float M10, M11, M12, M13;
			float M20, M21, M22, M23;
			float M30, M31, M32, M33;
		};
	};
}mat4x4, * mat4x4_ptr;

//--------------------------------
void onLoad();
void clear(u32 c);
void update(float dt);
void render(float dt);

void drawPoint(int x, int y, u32 c);
void drawLine(int x1, int y1, int x2, int y2, u32 c);
void DDA(int x, int y, int w, int h, u32 c);
void bresenham2(int x1, int y1, int x2, int y2, u32 c);
void drawRect(int x, int y, int w, int h, u32 c);
void drawTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c);
void fillTopFlatTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c);
void fillBottomFlatTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c);
void fillTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c);

void fpsCounting();
void MultiplyMatrixVector(mat4x4& m, vec3& i, vec3& o);
int __cdecl printf2(const char* format, ...);
uint64_t timeSinceEpochMillisec();

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN
//-----------------------------------------------
// global
const float PI = 3.14159265358979f;

bool running = true;

void* buffer_memory;
int buffer_width, buffer_height;
BITMAPINFO buffer_bitmap_info;

//-----------------------------------------------
// just test

u32 backcolor = 0xFFFFFF;//0xAARRGGBB;

vec3 v1{ 200,300 }, v2{ 250,400 }, v3{ 100,350 };

mesh meshCube;
int scale = 100;
float fl = 3.5, centZ = 0;
float fTheta = 0;

mat4x4 matProj;
int lastT = (int)timeSinceEpochMillisec(), dt = 0, totalT = 0;;
int fps = 0, frames = 0;
//-----------------------------------------------

void onLoad() {
	meshCube = {
		{
			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },

			// EAST                                                      
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },

			// NORTH                                                     
			{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },

			// WEST                                                      
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },

			// TOP                                                       
			{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },

			// BOTTOM                                                    
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }
		}
	};

	float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFov = 90.0f;
	float fAspectRatio = (float)buffer_height / (float)buffer_width;
	float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

	matProj = {
		fAspectRatio * fFovRad,0,0,0,
		0,fFovRad,0,0,
		0,0,fFar / (fFar - fNear),(-fFar * fNear) / (fFar - fNear),
		0,0,1,0
	};
}

void update(float dt) {

	//printf2("x:%f, y:%f\n", rect.x, rect.y);
}

void render(float dt) {
	clear(backcolor);
	// -----------
	fillTriangle(v1, v2, v3, 0xFF7700);
	drawTriangle(v1, v2, v3, 0x000000);

	mat4x4 matRotZ, matRotX, matScale;
	fTheta += 0.7f * dt / 1000;

	// Rotation Z
	matRotZ = {
		cosf(fTheta),-sinf(fTheta),0,0,
		sinf(fTheta),cosf(fTheta),0,0,
		0,0,1,0,
		0,0,1,0
	};

	// Rotation X
	matRotX = {
		1,0,0,0,
		0,cosf(fTheta * 0.5f),-sinf(fTheta * 0.5f),0,
		0,sinf(fTheta * 0.5f),cosf(fTheta * 0.5f),0,
		0,0,1,0
	};
	for (auto tri : meshCube.tris)
	{
		triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

		// Rotate in Z-Axis
		MultiplyMatrixVector(matRotZ, tri.p[0], triRotatedZ.p[0]);
		MultiplyMatrixVector(matRotZ, tri.p[1], triRotatedZ.p[1]);
		MultiplyMatrixVector(matRotZ, tri.p[2], triRotatedZ.p[2]);

		// Rotate in X-Axis
		MultiplyMatrixVector(matRotX, triRotatedZ.p[0], triRotatedZX.p[0]);
		MultiplyMatrixVector(matRotX, triRotatedZ.p[1], triRotatedZX.p[1]);
		MultiplyMatrixVector(matRotX, triRotatedZ.p[2], triRotatedZX.p[2]);

		// Offset into the screen
		triTranslated = triRotatedZX;
		triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
		triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
		triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

		// Project triangles from 3D --> 2D
		MultiplyMatrixVector(matProj, triTranslated.p[0], triProjected.p[0]);
		MultiplyMatrixVector(matProj, triTranslated.p[1], triProjected.p[1]);
		MultiplyMatrixVector(matProj, triTranslated.p[2], triProjected.p[2]);

		// Scale into view
		triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
		triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
		triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
		triProjected.p[0].x *= 0.5f * (float)buffer_width;
		triProjected.p[0].y *= 0.5f * (float)buffer_height;
		triProjected.p[1].x *= 0.5f * (float)buffer_width;
		triProjected.p[1].y *= 0.5f * (float)buffer_height;
		triProjected.p[2].x *= 0.5f * (float)buffer_width;
		triProjected.p[2].y *= 0.5f * (float)buffer_height;

		// Rasterize triangle
		drawTriangle(triProjected.p[0], triProjected.p[1], triProjected.p[2], 0x0);
	}

}


void drawPoint(int x, int y, u32 c) {
	if (x < 0 || x >= buffer_width || y < 0 || y >= buffer_height)return;
	u32* pixel = (u32*)buffer_memory;
	int pos = buffer_width * (buffer_height - 1 - y) + x;
	*(pixel + pos) = (u32)c;
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
void drawTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c) {
	drawLine(v1.x, v1.y, v2.x, v2.y, c);
	drawLine(v2.x, v2.y, v3.x, v3.y, c);
	drawLine(v3.x, v3.y, v1.x, v1.y, c);
}
void fillTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c) {
	if (v1.y > v2.y)std::swap(v1, v2);
	if (v2.y > v3.y)std::swap(v2, v3);
	if (v1.y > v2.y)std::swap(v1, v2);

	if (v2.y == v3.y)
	{
		fillBottomFlatTriangle(v1, v2, v3, c);
	}
	else if (v1.y == v2.y)
	{
		fillTopFlatTriangle(v1, v2, v3, c);
	}
	else
	{
		vec3 v4{ (v1.x + ((v2.y - v1.y) / (v3.y - v1.y)) * (v3.x - v1.x)), v2.y };
		fillBottomFlatTriangle(v1, v2, v4, c);
		fillTopFlatTriangle(v2, v4, v3, c);
	}
}

void fillBottomFlatTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c) {
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
void fillTopFlatTriangle(vec3 v1, vec3 v2, vec3 v3, u32 c) {
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

//------------------------------------------------------------

uint64_t timeSinceEpochMillisec() {
	//using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
void fpsCounting() {
	dt = timeSinceEpochMillisec() - lastT;
	totalT += dt;
	frames++;
	//printf("%d\n", totalT);
	if (totalT >= 1000) {
		totalT -= 1000;
		//printf2("FPS:%d\n", fpsCount);
		fps = frames;
		frames = 0;
	}
	lastT = timeSinceEpochMillisec();
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

void MultiplyMatrixVector(mat4x4& m44, vec3& i, vec3& o)
{
	/*
	r\c
		[a b c d]
		[e f g h]
		[i j k l]
		[m44 n o p]
	m44[0][1]=>b
	m44[0][2]=>c
	*/
	o.x = i.x * m44.M[0][0] + i.y * m44.M[0][1] + i.z * m44.M[0][2] + m44.M[0][3];
	o.y = i.x * m44.M[1][0] + i.y * m44.M[1][1] + i.z * m44.M[1][2] + m44.M[1][3];
	o.z = i.x * m44.M[2][0] + i.y * m44.M[2][1] + i.z * m44.M[2][2] + m44.M[2][3];
	float w = i.x * m44.M[3][0] + i.y * m44.M[3][1] + i.z * m44.M[3][2] + m44.M[3][3];

	if (w != 0.0f)
	{
		o.x /= w; o.y /= w; o.z /= w;
	}
}




//-----------------------------------------------

// Entry point
int main() {
	WCHAR cls_Name[] = L"Win32 Class";
	WNDCLASS wc = { };

	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = cls_Name;
	wc.hInstance = GetModuleHandle(NULL);
	//wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&wc);

	mat4x4 m44 = {
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15
	};

	// create
	HWND hwnd = CreateWindow(
		cls_Name,			//類別名稱，和剛剛一致
		L"Win32 test",		//標題文字
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, //視窗外觀樣式
		100,				//相對於parent的x座標
		200,				//相對於parent的y座標
		800 + 16,				//視窗寬度
		600 + 39,				//視窗高度
		NULL,				//parent window，沒的話NULL即可
		NULL,				//menu
		wc.hInstance,			//當前程式的追蹤
		NULL);				//附加資料
	HDC hdc = GetDC(hwnd);

	if (hwnd == NULL) //判斷是否成功Create
		return 0;

	// 顯示視窗 dxstyle =| WS_VISIBLE 就不用這行
	//ShowWindow(hwnd, SW_SHOW);

	onLoad();
	wchar_t strBuffer[100];
	// 事件訊息loop
	while (running) {
		fpsCounting();
		swprintf(strBuffer, 100, L"Win32 Render %dx%d, FPS:%4d, dt:%2dms", buffer_width, buffer_height, fps, dt);
		SetWindowText(hwnd, strBuffer);
		// Input
		MSG msg;
		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Update
		update(dt);
		render(dt);

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
		case WM_SIZE: { // when window change
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
		case WM_KEYDOWN:
			break;
		case WM_KEYUP:
			break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}