#include "device.h"

//-----------------------------------------
void onLoad();
void gameMain();
//-----------------------------------------
device_t device;
//-----------------------------------------
int main(void)
{
	const TCHAR* title = _T("Win32");

	if (screen_init(800, 600, title))
		return -1;

	device_init(&device, 800, 600, screen_fb);

	while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0) {
		screen_dispatch();

		gameMain();

		screen_update();
		Sleep(1);
	}
	return 0;
}

void onLoad() {

}


int sx = 10, sy = 10;
void gameMain() {

	printf("%s\n", "is running");
	device_clear(&device, 0);

	device_draw_line(&device, sx, sy, sx + 300, sy + 300, 0x000000);

	device_draw_line(&device, sx, sy, sx + 300, sy + 0, 0x0000FF);
	device_draw_line(&device, sx, sy + 1, sx + 300, sy + 1, 0x0000FF);

	device_draw_line(&device, sx, sy, sx + 0, sy + 300, 0xFF0000);
	device_draw_line(&device, sx + 1, sy, sx + 1, sy + 300, 0xFF0000);
}
