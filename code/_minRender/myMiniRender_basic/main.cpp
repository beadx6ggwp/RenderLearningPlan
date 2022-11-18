#include <iostream>
#include <cmath>

#include "device.h"
#include "screen.h"

#include "tool.h"
#include "geometry.h"
//using namespace std;
//-----------------------------------------
void onLoad();
void gameMain();
void update(float deltatime);
void render();
//-----------------------------------------
const int width = 600, height = 600, depth = 255;
Device device;
//--------------------------------------------------------


Vec2f player;
Vec2f speed;
int playerW = 30, playerH = 50;

int main(void) {
	const TCHAR* title = _T("Win32");
	if (screen_init(width, height, title))
		return -1;

	device.init(width, height, screen_fb);
	device.background = rgb2hex(0.85 * 255, 0.85 * 255, 1 * 255); // set background color

	onLoad();
	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch(); // system_event

		gameMain();

		screen_update();// show framebuffer to screen
		//Sleep(1);// 1~n ms, if running too fast
	}
	return 0;
}

void onLoad() {
	std::cout << "Start\n";

	player = Vec2f(10, 20);
	speed = Vec2f(400, 200);
}
void gameMain() {
	fpsCounting();
	showFps(device);

	update(deltaT / 1000.0f);

	device.clear();
	render();
}

void update(float dt) {
	//cout << deltatime << "\n";
	if (screen_keys[VK_UP]) {
		std::cout << "press up\n";
	}
	if (screen_keys['W']) {
		std::cout << "press w\n";
	}

	//------------------

	player.x += speed.x * dt;
	player.y += speed.y * dt;

	if (player.x > width || player.x < 0)speed.x *= -1;
	if (player.y > width || player.y < 0)speed.y *= -1;
}

int testx = 100;
void render() {
	// draw player
	device.fillTriangle2(
		Vec3f(player.x, player.y, 0),
		Vec3f(player.x - playerW, player.y + playerH, 0),
		Vec3f(player.x + playerW, player.y + playerH, 0),
		0xff0000);


	//--test--
	// move 1 pixel each frame
	testx = (testx + 1) % device.width;
	for (int i = 0; i < 10; i++)
		device.setPixel(testx - i, 10, 0x000000);

	// draw cross, color format: 0xRRGGBB
	device.drawLine(400, 100, 500, 200, 0xff0000);// red
	device.drawLine(500, 100, 400, 200, 0x0000ff);// blue

	// draw triangle & wire frame
	device.fillTriangle2(Vec3f(300, 100, 0), Vec3f(250, 200, 0), Vec3f(350, 190, 0), 0xff7700);
	device.drawTriangle(Vec3f(300, 100, 0), Vec3f(250, 200, 0), Vec3f(350, 190, 0), 0x000000);
}