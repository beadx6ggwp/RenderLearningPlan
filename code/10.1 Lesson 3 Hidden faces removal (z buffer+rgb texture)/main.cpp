#include <iostream>
#include <cmath>
#include <limits>

#include "tool.h"
#include "geometry.h"

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
		//Sleep(1);
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

	//device.drawLine(0, 300, 200, 400, 0xff0000);
	device.filltriangle_bery({ 0,0 }, { 80,0 }, { 0,40 }, 0xff7700);

	device.filltriangle_bery_zbuffer({ 50,100,0 }, { 200,50,0 }, { 200,150,0 }, 0xff0000);
	device.filltriangle_bery_zbuffer({ 90,40,50 }, { 90,160,50 }, { 150,100,100 }, 0x00ff00);
	device.filltriangle_bery_zbuffer({ 100,100,100 }, { 180,20,-50 }, { 180,180,-50 }, 0x0000ff);

	// clock-wise mapping
	vec3 cols[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
	device.filltriangle_bery_testRGB({ 10,300,0 }, { 170,300,0 }, { 90,450,0 }, cols);


	vec3 cols2[3] = { {1, 1, 0}, {0, 1, 1}, {1, 0, 1} };
	device.filltriangle_bery_testRGB({ 10,300 + 200,0 }, { 170,300 + 200,0 }, { 90,450 + 200,0 }, cols2);

	vec3 cols3[3] = { {1, 0, 0}, {1, 0, 0}, {0, 0, 1} };
	//device.filltriangle_bery_testRGB({ 10,300 + 200,0 }, { 170,300 + 200,0 }, { 90,450 + 200,0 }, cols3);



	vec3 light = { 0,0,-1 };
	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		vec3 screen_coords[3], world_coords[3];

		for (int j = 0; j < 3; j++) {
			vec3 v = model->vert(face[j]);
			// 反轉Y->平移->縮放置螢幕大小
			screen_coords[j] = { (v.x + 1.f) * device.width / 2.f, (-v.y + 1.f) * device.height / 2.f, v.z };
			world_coords[j] = v;
		}

		vec3 n, line1, line2;
		line1 = Vector_Sub(world_coords[2], world_coords[0]);
		line2 = Vector_Sub(world_coords[1], world_coords[0]);
		n = Vector_CrossProduct(line1, line2);
		n = Vector_Normalise(n);
		float intensity = Vector_DotProduct(n, light);
		if (intensity > 0) {
			device.filltriangle_bery_zbuffer(screen_coords[0], screen_coords[1], screen_coords[2], rgb2hex(intensity * 255, intensity * 255, intensity * 255));
		}
	}
}