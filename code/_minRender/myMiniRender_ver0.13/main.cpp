#include <iostream>
#include <cmath>
#include <limits>
#include <map>
#include <iomanip>

#include "tool.h"
#include "geometry.h"
#include "tgaimage.h"

#include "screen.h"
#include "device.h"
#include "model.h"
#include "my_gl.h"

//using namespace std;
//-----------------------------------------
void onLoad();
void gameMain();
void update(float deltatime);
void render();
//-----------------------------------------
const int width = 800, height = 800, depth = 255;
Device device;
//--------------------------------------------------------
//UI32 texture[256][256];

void testCase();

int main(void) {
	const TCHAR* title = _T("Win32");
	if (screen_init(width, height, title))
		return -1;

	device.init(width, height, screen_fb);
	device.background = rgb2hex(0.85 * 255, 0.85 * 255, 1 * 255); // set background color

	onLoad();
	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch(); // event

		gameMain();

		screen_update();// show framebuffer to screen
		//Sleep(1);
	}
	return 0;
}

void onLoad() {
	std::cout << "Matrix test:\n";
	Matrix testmat = Matrix::identity();
	testmat.set_col(0, embed<4>(Vec3f(1, 2, 3), 5.0f));
	std::cout << testmat;

	// create blue/white texture
	int i, j;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int x = i / 32, y = j / 32;
			device.bery_texture[j][i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
		}
	}
}
void gameMain() {
	fpsCounting();
	swprintf(strBuffer, 100,
			 L"Render ver0.1, %dx%d, FPS:%4d, dt:%2dms",
			 device.width, device.height, fps, deltaT);
	SetWindowText(screen_handle, strBuffer); // change title bar

	update(deltaT / 1000.0f);
	render();
}

void update(float deltatime) {
	// deltatime: https://gafferongames.com/post/integration_basics/
	//cout << deltatime << "\n";

	if (screen_keys[VK_UP]) {
		std::cout << "press up\n";
	}
	if (screen_keys['W']) {
		std::cout << "press w\n";
	}
}

void render() {
	device.clear();

	// draw cross, color format: 0xRRGGBB
	device.drawLine(400, 100, 500, 200, 0xff0000);
	device.drawLine(500, 100, 400, 200, 0x0000ff);

	// draw tri with Outer frame
	device.fillTriangle2(Vec3f(300, 100, 0), Vec3f(250, 200, 0), Vec3f(350, 190, 0), 0xff7700);
	device.drawTriangle(Vec3f(300, 100, 0), Vec3f(250, 200, 0), Vec3f(350, 190, 0), 0);

	// show test case
	//testCase();
}

void testCase() {
	//device.drawLine(0, 300, 200, 400, 0xff0000);
	Vec3f triTest[] = { Vec3f(0,0,0), Vec3f(80,0,0), Vec3f(0,40,0) };
	device.filltriangle_bery(triTest, 0xff7700);

	// z-buffer穿插測試
	Vec3f triR[] = { Vec3f(50,100,0), Vec3f(200,50,0), Vec3f(200,150,0) };
	device.filltriangle_bery_zbuffer(triR, 0xff0000);
	Vec3f triG[] = { Vec3f(90,40,50), Vec3f(90,160,50), Vec3f(150,100,100) };
	device.filltriangle_bery_zbuffer(triG, 0x00ff00);
	Vec3f triB[] = { Vec3f(100,100,100), Vec3f(180,20,-50), Vec3f(180,180,-50) };
	device.filltriangle_bery_zbuffer(triB, 0x0000ff);

	// clock-wise RGB mapping
	Vec3f cols[] = { Vec3f(1,0,0), Vec3f(0,1,0), Vec3f(0,0,1) };
	Vec3f a(10, 200, 0);
	Vec3f tri1[] = { a, Vec3f(a.x + 200,a.y,a.z), Vec3f(a.x + 100,a.y + 150,a.z) };
	device.filltriangle_bery_testRGB(tri1, cols);

	a.y += 200;
	Vec3f cols2[] = { Vec3f(1,1,0), Vec3f(0,1,1), Vec3f(1,0,1) };
	Vec3f tri2[] = { a, Vec3f(a.x + 200,a.y,a.z), Vec3f(a.x + 100,a.y + 150,a.z) };
	device.filltriangle_bery_testRGB(tri2, cols2);

	a.y += 200;
	Vec3f cols3[] = { Vec3f(1,0,0), Vec3f(1,0,0), Vec3f(0,0,1) };
	Vec3f tri3[] = { a, Vec3f(a.x + 200,a.y,a.z), Vec3f(a.x + 100,a.y + 150,a.z) };
	device.filltriangle_bery_testRGB(tri3, cols3);

	//texture test
	Vec3f p1(300, 300, 0), size(200, 200, 0);
	Vec3f vt1[] = { Vec3f(0, 0, 0), Vec3f(1, 0, 0), Vec3f(0, 1, 0) };
	Vec3f triT1[] = { p1, Vec3f(p1.x + size.x,p1.y,0), Vec3f(p1.x,p1.y + size.y,0) };
	device.filltriangle_bery_texture(triT1, vt1);

	p1.x += 3; p1.y += 3;
	Vec3f vt2[] = { Vec3f(1, 0, 0), Vec3f(1, 1, 0), Vec3f(0, 1, 0) };
	Vec3f triT2[] = { Vec3f(p1.x + size.x,p1.y,0), Vec3f(p1.x + size.x,p1.y + size.y,0), Vec3f(p1.x,p1.y + size.y,0) };
	device.filltriangle_bery_texture(triT2, vt2);
}

