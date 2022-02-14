#include <iostream>

#include "geometry.h"

#include "tool.h"
#include "screen.h"
#include "device.h"
#include "model.h"

using namespace std;
//-----------------------------------------
void onLoad();
void gameMain();
void update(float deltatime);
void render();
//-----------------------------------------
const int width = 800, height = 800;
Device device;

Model* model;

int main(void) {

	const TCHAR* title = _T("Win32");

	if (screen_init(width, height, title))
		return -1;

	device.init(width, height, screen_fb);
	device.background = 0x0;

	model = new Model("african_head.obj");

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch(); // event

		gameMain();

		screen_update();// swow framebuffer to screen
		Sleep(1);
	}
	return 0;
}

void onLoad() {

}
void gameMain() {
	fpsCounting();
	swprintf(strBuffer, 100,
			 L"Render ver0.1, %dx%d, FPS:%4d, dt:%2dms",
			 device.width, device.height, fps, dt);
	SetWindowText(screen_handle, strBuffer);
	update(dt / 1000.0f);
	render();
}

void update(float deltatime) {
	//cout << deltatime << "\n";
}

void render() {
	device.clear();

	device.drawLine(0, 0, 200, 100, 0xff0000);

	// just show x,y
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		for (int j = 0; j < 3; j++) {
			vec3 v0 = model->vert(face[j]);
			vec3 v1 = model->vert(face[(j + 1) % 3]);
			// 反轉Y->平移->縮放置螢幕大小
			int x0 = (v0.x + 1.) * device.width / 2.;
			int y0 = (-v0.y + 1.) * device.height / 2.;
			int x1 = (v1.x + 1.) * device.width / 2.;
			int y1 = (-v1.y + 1.) * device.height / 2.;
			device.drawLine(x0, y0, x1, y1, 0xffffff);
		}
	}
}