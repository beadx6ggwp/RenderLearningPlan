#include "tool.h"



wchar_t strBuffer[100];
int lastT = (int)timeSinceEpochMillisec(), dt = 0, totalT = 0;;
int fps = 0, frames = 0;

uint64_t timeSinceEpochMillisec() {
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

UI32 rgb2hex(UI32 r, UI32 g, UI32 b) {
	return r << 16 | g << 8 | b;
}


vec3 hex2rgb(UI32 c) {
	// x=r, y=g, z=b;
	vec3 v = { c >> 16,c >> 8 & 0xff, c & 0xff };
	return v;
}