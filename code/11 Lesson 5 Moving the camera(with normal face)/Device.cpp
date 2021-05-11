#include "Device.h"
#include "Geometry.h"

// 擴充的繪製功能

// https://www.avrfreaks.net/sites/default/files/triangles.c
void Device::fillTriangle2(vec3 v1, vec3 v2, vec3 v3, UI32 c)
{
	auto SWAP = [](int& x, int& y) { int t = x; x = y; y = t; };
	auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) setPixel(i, ny, c); };
	int x1 = v1.x, y1 = v1.y;
	int x2 = v2.x, y2 = v2.y;
	int x3 = v3.x, y3 = v3.y;
	int t1x, t2x, y, minx, maxx, t1xp, t2xp;
	bool changed1 = false;
	bool changed2 = false;
	int signx1, signx2, dx1, dy1, dx2, dy2;
	int e1, e2;
	// Sort vertices
	if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
	if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
	if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

	t1x = t2x = x1; y = y1;   // Starting points
	dx1 = (int)(x2 - x1); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (int)(y2 - y1);

	dx2 = (int)(x3 - x1); if (dx2 < 0) { dx2 = -dx2; signx2 = -1; }
	else signx2 = 1;
	dy2 = (int)(y3 - y1);

	if (dy1 > dx1) {   // swap values
		SWAP(dx1, dy1);
		changed1 = true;
	}
	if (dy2 > dx2) {   // swap values
		SWAP(dy2, dx2);
		changed2 = true;
	}

	e2 = (int)(dx2 >> 1);
	// Flat top, just process the second half
	if (y1 == y2) goto next;
	e1 = (int)(dx1 >> 1);

	for (int i = 0; i < dx1;) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			i++;
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) t1xp = signx1;//t1x += signx1;
				else          goto next1;
			}
			if (changed1) break;
			else t1x += signx1;
		}
		// Move line
	next1:
		// process second line until y value is about to change
		while (1) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;//t2x += signx2;
				else          goto next2;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next2:
		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);    // Draw line from min to max points found on the y
									 // Now increase y
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y == y2) break;

	}
next:
	// Second half
	dx1 = (int)(x3 - x2); if (dx1 < 0) { dx1 = -dx1; signx1 = -1; }
	else signx1 = 1;
	dy1 = (int)(y3 - y2);
	t1x = x2;

	if (dy1 > dx1) {   // swap values
		SWAP(dy1, dx1);
		changed1 = true;
	}
	else changed1 = false;

	e1 = (int)(dx1 >> 1);

	for (int i = 0; i <= dx1; i++) {
		t1xp = 0; t2xp = 0;
		if (t1x < t2x) { minx = t1x; maxx = t2x; }
		else { minx = t2x; maxx = t1x; }
		// process first line until y value is about to change
		while (i < dx1) {
			e1 += dy1;
			while (e1 >= dx1) {
				e1 -= dx1;
				if (changed1) { t1xp = signx1; break; }//t1x += signx1;
				else          goto next3;
			}
			if (changed1) break;
			else   	   	  t1x += signx1;
			if (i < dx1) i++;
		}
	next3:
		// process second line until y value is about to change
		while (t2x != x3) {
			e2 += dy2;
			while (e2 >= dx2) {
				e2 -= dx2;
				if (changed2) t2xp = signx2;
				else          goto next4;
			}
			if (changed2)     break;
			else              t2x += signx2;
		}
	next4:

		if (minx > t1x) minx = t1x; if (minx > t2x) minx = t2x;
		if (maxx < t1x) maxx = t1x; if (maxx < t2x) maxx = t2x;
		drawline(minx, maxx, y);
		if (!changed1) t1x += signx1;
		t1x += t1xp;
		if (!changed2) t2x += signx2;
		t2x += t2xp;
		y += 1;
		if (y > y3) return;
	}
}

void Device::drawTriangle(vec3 v1, vec3 v2, vec3 v3, UI32 c) {
	drawLine(v1.x, v1.y, v2.x, v2.y, c);
	drawLine(v2.x, v2.y, v3.x, v3.y, c);
	drawLine(v3.x, v3.y, v1.x, v1.y, c);
}


void Device::filltriangle_bery(vec3 p1, vec3 p2, vec3 p3, UI32 c) {
	vec3 bboxMin = {
		std::floor(std::min(p1.x, std::min(p2.x, p3.x))),
		std::floor(std::min(p1.y, std::min(p2.y, p3.y)))
	};
	vec3 bboxMax = {
		std::ceil(std::max(p1.x, std::max(p2.x, p3.x))),
		std::ceil(std::max(p1.y, std::max(p2.y, p3.y)))
	};
	vec3 p, bc_screen;
	for (p.x = bboxMin.x; p.x <= bboxMax.x; p.x++) {
		for (p.y = bboxMin.y; p.y <= bboxMax.y; p.y++) {
			vec3 bc_screen = barycentric(p1, p2, p3, p);
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			setPixel(p.x, p.y, c);
		}
	}
}
void Device::filltriangle_bery_zbuffer(vec3 p1, vec3 p2, vec3 p3, UI32 c) {
	vec3 bboxMin = {
		std::floor(std::min(p1.x, std::min(p2.x, p3.x))),
		std::floor(std::min(p1.y, std::min(p2.y, p3.y)))
	};
	vec3 bboxMax = {
		std::ceil(std::max(p1.x, std::max(p2.x, p3.x))),
		std::ceil(std::max(p1.y, std::max(p2.y, p3.y)))
	};
	vec3 p, bc_screen;
	for (p.x = bboxMin.x; p.x <= bboxMax.x; p.x++) {
		for (p.y = bboxMin.y; p.y <= bboxMax.y; p.y++) {
			bc_screen = barycentric(p1, p2, p3, p);
			// check point in tri
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			vec3 vz = { p1.z,p2.z,p3.z };
			p.z = Vector_DotProduct(vz, bc_screen);
			if (p.z > zbuffer[(int)p.y][(int)p.x]) {
				zbuffer[(int)p.y][(int)p.x] = p.z;
				//UI32 zbuffer_gray = p.z + 50;
				//setPixel(p.x, p.y, zbuffer_gray | zbuffer_gray << 8 | zbuffer_gray <<16);
				setPixel(p.x, p.y, c);
			}
		}
	}
}


void Device::filltriangle_bery_testRGB(vec3 p1, vec3 p2, vec3 p3, vec3 colors[]) {
	vec3 bboxMin = {
		std::floor(std::min(p1.x, std::min(p2.x, p3.x))),
		std::floor(std::min(p1.y, std::min(p2.y, p3.y)))
	};
	vec3 bboxMax = {
		std::ceil(std::max(p1.x, std::max(p2.x, p3.x))),
		std::ceil(std::max(p1.y, std::max(p2.y, p3.y)))
	};
	vec3 p, bc_screen;
	for (p.x = bboxMin.x; p.x <= bboxMax.x; p.x++) {
		for (p.y = bboxMin.y; p.y <= bboxMax.y; p.y++) {
			bc_screen = barycentric(p1, p2, p3, p);
			// check point in tri
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
			vec3 vz = { p1.z,p2.z,p3.z };
			p.z = Vector_DotProduct(vz, bc_screen);
			if (p.z > zbuffer[(int)p.y][(int)p.x]) {
				zbuffer[(int)p.y][(int)p.x] = p.z;

				vec3 col = {
					bc_screen.x * colors[0].x + bc_screen.y * colors[1].x + bc_screen.z * colors[2].x,
					bc_screen.x * colors[0].y + bc_screen.y * colors[1].y + bc_screen.z * colors[2].y,
					bc_screen.x * colors[0].z + bc_screen.y * colors[1].z + bc_screen.z * colors[2].z
				};
				UI32 c = (int)(col.x * 255) << 16 | (int)(col.y * 255) << 8 | (int)(col.z * 255);
				setPixel(p.x, p.y, c);
			}
		}
	}
}
