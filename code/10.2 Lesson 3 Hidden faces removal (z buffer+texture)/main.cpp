﻿#include <iostream>
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

UI32 texture[256][256];
UI32 texture2[16][16] = {
		{0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xFF0000,0xFF0000,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xFF0000,0xFF0000,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xFF0000,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff},
		{0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff,0xffffff}
};
/*
UI32 texture2[16][16] = {
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000},
		{0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000}
};
*/

int main(void) {

	const TCHAR* title = _T("Win32");

	if (screen_init(width, height, title))
		return -1;

	device.init(width, height, screen_fb);
	device.background = 0x0;


	onLoad();
	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch(); // event

		gameMain();

		screen_update();// swow framebuffer to screen
		//Sleep(1);
	}
	return 0;
}

void onLoad() {
	model = new Model("african_head.obj");

	int i, j;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int x = i / 32, y = j / 32;
			texture[j][i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
		}
	}
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
void filltriangle_bery_texture(vec3 p1, vec3 p2, vec3 p3, vec3 vt[]);
void render() {
	device.clear();

	//device.drawLine(0, 300, 200, 400, 0xff0000);
	device.filltriangle_bery({ 0,0 }, { 80,0 }, { 0,40 }, 0xff7700);

	device.filltriangle_bery_zbuffer({ 50,100,0 }, { 200,50,0 }, { 200,150,0 }, 0xff0000);
	device.filltriangle_bery_zbuffer({ 90,40,50 }, { 90,160,50 }, { 150,100,100 }, 0x00ff00);
	device.filltriangle_bery_zbuffer({ 100,100,100 }, { 180,20,-50 }, { 180,180,-50 }, 0x0000ff);

	// clock-wise mapping
	vec3 cols[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
	vec3 a = { 10 + 0,200 + 0,0 };
	device.filltriangle_bery_testRGB(a, { a.x + 200,a.y,a.z }, { a.x + 100,a.y + 150,a.z }, cols);


	vec3 cols2[3] = { {1, 1, 0}, {0, 1, 1}, {1, 0, 1} };
	device.filltriangle_bery_testRGB({ 10,300 + 100,0 }, { 170,300 + 100,0 }, { 90,450 + 100,0 }, cols2);

	vec3 cols3[3] = { {1, 0, 0}, {1, 0, 0}, {0, 0, 1} };
	device.filltriangle_bery_testRGB({ 10,300 + 300,0 }, { 170,300 + 300,0 }, { 90,450 + 300,0 }, cols3);

	vec3 p1 = { 300,300,0 }, size = { 200,200,0 };
	vec3 vt1[3] = { {0, 0, 0}, {1, 0, 0}, {0, 1, 0} };
	filltriangle_bery_texture(p1, { p1.x + size.x,p1.y,0 }, { p1.x,p1.y + size.y,0 }, vt1);
	p1.x += 2; p1.y += 2;
	vec3 vt2[3] = { {1, 0, 0}, {1, 1, 0}, {0, 1, 0} };
	filltriangle_bery_texture({ p1.x + size.x,p1.y,0 }, { p1.x + size.x,p1.y + size.y,0 }, { p1.x,p1.y + size.y,0 }, vt2);
}



void filltriangle_bery_texture(vec3 p1, vec3 p2, vec3 p3, vec3 vt[]) {
	vec3 bboxMin = {
		std::floor((std::min)(p1.x, (std::min)(p2.x, p3.x))),
		std::floor((std::min)(p1.y, (std::min)(p2.y, p3.y)))
	};
	vec3 bboxMax = {
		std::ceil(((std::max))(p1.x, (std::max)(p2.x, p3.x))),
		std::ceil((std::max)(p1.y, (std::max)(p2.y, p3.y)))
	};
	vec3 p, bc_screen;
	for (p.x = bboxMin.x; p.x <= bboxMax.x; p.x++) {
		for (p.y = bboxMin.y; p.y <= bboxMax.y; p.y++) {
			bc_screen = barycentric(p1, p2, p3, p);
			// check point in tri
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			vec3 vz = { p1.z,p2.z,p3.z };
			p.z = Vector_DotProduct(vz, bc_screen);
			if (p.z > device.zbuffer[(int)p.y][(int)p.x]) {
				device.zbuffer[(int)p.y][(int)p.x] = p.z;

				vec3 col = {
					bc_screen.x * vt[0].x + bc_screen.y * vt[1].x + bc_screen.z * vt[2].x,
					bc_screen.x * vt[0].y + bc_screen.y * vt[1].y + bc_screen.z * vt[2].y,
					bc_screen.x * vt[0].z + bc_screen.y * vt[1].z + bc_screen.z * vt[2].z
				};
				int u = col.x * 255;
				int v = col.y * 255;
				UI32 c = texture[v][u];
				//int u = col.x * 16;
				//int v = col.y * 16;
				//UI32 c = texture2[v][u];
				device.setPixel(p.x, p.y, c);
			}
		}
	}
}