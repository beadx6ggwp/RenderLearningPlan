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
typedef struct _Mat4x4 { float m[4][4] = { 0 }; } mat4x4;
typedef struct _Vec2 { float x, y; } vec2;
typedef struct _Vec3 { float x, y, z; } vec3;

typedef struct _Triangle { vec3 p[3]; } triangle;
typedef struct _Mesh { vector<triangle> tris; } mesh;

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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//MSDN
int __cdecl printf2(const char* format, ...);

uint64_t timeSinceEpochMillisec();
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

mesh* meshs;
vec2* poss;
vec3 offset{ 300,300,0 };
int scale = 100;
float fl = 3.5, centZ = 0;
float fTheta = 0;


uint64_t timeSinceEpochMillisec() {
	//using namespace std::chrono;
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
int lastT = timeSinceEpochMillisec(), dt = 0, totalT = 0;;
int fps = 0, frames = 0;
//-----------------------------------------------

void onLoad() {
	mesh meshCube{
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

	mesh meshTri{
		{
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f }
		}
	};
	mesh meshRect{
		{
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f }
		}
	};

	mesh meshCones{
		{
			//bot
			{ 0.0f, 0.0f, 0.0f,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f },

			// SOUTH
			{ 0.0f, 0.0f, 0.0f,    0.5f, 1.2, 0.5f,    1.0f, 0.0f, 0.0f },

			// EAST                                                      
			{ 1.0f, 0.0f, 0.0f,    0.5f, 1.2, 0.5f,    1.0f, 0.0f, 1.0f },

			// NORTH                                                     
			{ 1.0f, 0.0f, 1.0f,    0.5f, 1.2, 0.5f,    0.0f, 0.0f, 1.0f },

			// WEST                                                      
			{ 0.0f, 0.0f, 1.0f,    0.5f, 1.2, 0.5f,    0.0f, 0.0f, 0.0f }
		}
	};

	meshs = new mesh[]{ meshTri, meshRect, meshCones,meshCube };
	poss = new vec2[]{ vec2{100,100},vec2{200,100},vec2{300,100},vec2{420,100} };
}

void update(float dt) {

	//printf2("x:%f, y:%f\n", rect.x, rect.y);
}

void render(float dt) {
	clear(backcolor);
	// -----------
	fillTriangle(v1, v2, v3, 0xFF7700);
	drawTriangle(v1, v2, v3, 0x000000);

	mat4x4 matRotZ, matRotX, matScale, matProj;
	fTheta += 0.7f * dt / 1000;

	// Rotation Z
	matRotZ.m[0][0] = cosf(fTheta);
	matRotZ.m[0][1] = -sinf(fTheta);
	matRotZ.m[1][0] = sinf(fTheta);
	matRotZ.m[1][1] = cosf(fTheta);
	matRotZ.m[2][2] = 1;
	matRotZ.m[3][3] = 1;

	// Rotation X
	matRotX.m[0][0] = 1;
	matRotX.m[1][1] = cosf(fTheta * 0.5f);
	matRotX.m[1][2] = -sinf(fTheta * 0.5f);
	matRotX.m[2][1] = sinf(fTheta * 0.5f);
	matRotX.m[2][2] = cosf(fTheta * 0.5f);
	matRotX.m[3][3] = 1;

	matScale.m[0][0] = 50;//sx
	matScale.m[1][1] = 50;//sy
	matScale.m[2][2] = 1;//sz
	matScale.m[0][3] = 0;//tx
	matScale.m[1][3] = 0;//ty
	matScale.m[2][3] = 0;//tz
	matScale.m[3][3] = 1;

	for (size_t i = 0; i < 4; i++) {
		matScale.m[0][3] = poss[i].x;//tx
		matScale.m[1][3] = poss[i].y;//ty
		for (auto tri : meshs[i].tris) {
			triangle triProjected, triTranslated, triRotatedZ, triRotatedZX, triRotated;
			// Rotate in Z-Axis
			for (size_t i = 0; i < 3; i++) MultiplyMatrixVector(matRotZ, tri.p[i], triRotatedZ.p[i]);
			// Rotate in X-Axis
			for (size_t i = 0; i < 3; i++) MultiplyMatrixVector(matRotX, triRotatedZ.p[i], triRotatedZX.p[i]);

			// Scale
			//for (size_t i = 0; i < 3; i++) MultiplyMatrixVector(matScale, triRotatedZX.p[i], triTranslated.p[i]);
			triRotated = triRotatedZX;

			// project tri from 3D->2D
			for (size_t i = 0; i < 3; i++) {
				float per = fl / (fl + triRotated.p[i].z + centZ);

				triProjected.p[i].x = triRotated.p[i].x * per;
				triProjected.p[i].y = triRotated.p[i].y * per;
			}
			for (size_t i = 0; i < 3; i++) MultiplyMatrixVector(matScale, triProjected.p[i], triTranslated.p[i]);
			if (i == 0)fillTriangle(triTranslated.p[0], triTranslated.p[1], triTranslated.p[2], 0xF0000);
			drawTriangle(triTranslated.p[0], triTranslated.p[1], triTranslated.p[2], 0x0000);
		}
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
		800 + 16,				//視窗寬度
		600 + 39,				//視窗高度
		NULL,				//parent window，沒的話NULL即可
		NULL,				//menu
		hInstance,			//當前程式的追蹤
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
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void fpsCounting() {
	dt = timeSinceEpochMillisec() - lastT;
	totalT += dt;
	frames++;
	//printf2("%d\n", totalT);
	if (totalT >= 1000) {
		totalT -= 1000;
		//printf2("FPS:%d\n", fpsCount);
		fps = frames;
		frames = 0;
	}
	lastT = timeSinceEpochMillisec();
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

void MultiplyMatrixVector(mat4x4& m, vec3& i, vec3& o)
{
	/*
	r\c
		[a b c d]
		[e f g h]
		[i j k l]
		[m n o p]
	m[0][1]=>b
	m[0][2]=>c
	*/
	o.x = i.x * m.m[0][0] + i.y * m.m[0][1] + i.z * m.m[0][2] + m.m[0][3];
	o.y = i.x * m.m[1][0] + i.y * m.m[1][1] + i.z * m.m[1][2] + m.m[1][3];
	o.z = i.x * m.m[2][0] + i.y * m.m[2][1] + i.z * m.m[2][2] + m.m[2][3];
	float w = i.x * m.m[3][0] + i.y * m.m[3][1] + i.z * m.m[3][2] + m.m[3][3];

	if (w != 0.0f)
	{
		o.x /= w; o.y /= w; o.z /= w;
	}
}
void setMat44(float a, float b, float c, float d,
			  float e, float f, float g, float h,
			  float i, float j, float k, float l,
			  float m2, float n, float o, float p,
			  mat4x4& m) {
	m.m[0][0] = a; m.m[0][1] = b; m.m[0][2] = c; m.m[0][3] = d;
	m.m[0][0] = e; m.m[0][1] = f; m.m[0][2] = g; m.m[0][3] = h;
	m.m[0][0] = i; m.m[0][0] = j; m.m[0][0] = k; m.m[0][0] = l;
	m.m[0][0] = m2; m.m[0][0] = n; m.m[0][0] = o; m.m[0][0] = p;
}