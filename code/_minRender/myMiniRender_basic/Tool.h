#pragma once

#include <chrono> // timer
#include "geometry.h"

using namespace std::chrono;

uint64_t timeSinceEpochMillisec() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

wchar_t strBuffer[100];
int lastT = (int)timeSinceEpochMillisec(), deltaT = 0, totalT = 0;;
int fps = 0, frames = 0;

void fpsCounting() {
	deltaT = timeSinceEpochMillisec() - lastT;
	totalT += deltaT;
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

void showFps(Device device) {
	swprintf(strBuffer, 100,
		L"Render ver0.1, %dx%d, FPS:%4d, dt:%2dms",
		device.width, device.height, fps, deltaT);
	SetWindowText(screen_handle, strBuffer); // change title bar
}



UI32 rgb2hex(UI32 r, UI32 g, UI32 b) {
	return r << 16 | g << 8 | b;
}

Vec3f hex2rgb(UI32 c) {
	// x=r, y=g, z=b;
	Vec3f v(c >> 16, c >> 8 & 0xff, c & 0xff);
	return v;
}