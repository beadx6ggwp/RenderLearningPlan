#include "device.h"

//-----------------------------------------
void onLoad();
void gameMain();
void update();
void render();
//-----------------------------------------
device_t device;
//-----------------------------------------
int main(void)
{
	const TCHAR* title = _T("Win32");

	if (screen_init(800, 600, title))
		return -1;

	device_init(&device, 800, 600, screen_fb);
	
	onLoad();
	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();

		gameMain();

		screen_update();
		Sleep(1);
	}
	return 0;
}

void onLoad() {
	printf("Start");
}

void gameMain() {
	update();
	render();
}

void update() {

}

void render() {
	device_clear(&device, 0);

	vec3 v1{ 200,300 }, v2{ 250,400 }, v3{ 100,350 };
	fillTriangle(&device, v1, v2, v3, 0xFF7700);
	drawTriangle(&device, v1, v2, v3, 0x000000);
}
