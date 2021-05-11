#include <iostream>
#include <cmath>
#include <limits>

#include "tool.h"
#include "geometry.h"
#include "tgaimage.h"

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
const int width = 800, height = 800, depth = 255;
Device device;

Model* model;

UI32 texture[256][256];

//--------------------------------------------------------
void triangle_tga_normal(vec3 t0, vec3 t1, vec3 t2, float ity0, float ity1, float ity2);
void triangle_tga_texture(vec3 t0, vec3 t1, vec3 t2, vec2 uv0, vec2 uv1, vec2 uv2, float intensity);

int* zbuffer = NULL;

vec3 light_dir = vec3{ 0, 0, -1 }.normalize();
vec3 eye{ 1,1,3 };
vec3 center{ 0, 0, 0 };

float fTheta = 0;
int isRot = 1;

//vec3 newP = Matrix_MultiplyVector(mvp, v);
//screen_coords[j] = { (newP.x + 1.f) * device.width / 2.f, (-newP.y + 1.f) * device.height / 2.f, newP.z };
mat4x4 viewport(int x, int y, int w, int h)
{
	/*
		x,y: offset
		w,h: screen scale size
	*/
	mat4x4 matrix = {
		w / 2.0f,		0,				0,				x + w / 2.0f,
		0,				-h / 2.0f,		0,				y + h / 2.0f,
		0,				0,				depth / 2.0f,	depth / 2.0f,
		0,				0,				0,				1
	};
	return matrix;
}

//https://stackoverflow.com/questions/17771406/c-initial-value-of-reference-to-non-const-must-be-an-lvalue
//http://www.songho.ca/opengl/gl_camera.html

mat4x4 lookat(vec3 eye, vec3 center, vec3 up)
{
	vec3 z = (eye - center).normalize();
	vec3 x = (up ^ z).normalize();
	vec3 y = (z ^ x).normalize();
	mat4x4 matrix = {
		x.x,				x.y,				x.z,				-center.x,
		y.x,				y.y,				y.z,				-center.y,
		z.x,				z.y,				z.z,				-center.z,
		0,					0,					0,					1
	};
	return matrix;
}


//--------------------------------------------------------

string dirPath = "../_objfile/testTexture/";
string objName = "dice.obj";
float scale = 0.2;
/*
african_head
rock
floor
diablo3_pose
wall1
capsule
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
	model = new Model(dirPath + objName);

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
	//--------------------------------------------------------------------------------------------------

	mat4x4 ModelView = lookat(eye, center, vec3{ 0, 1, 0 });
	mat4x4 Projection = Matrix_MakeIdentity();
	mat4x4 ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	Projection.M[3][2] = -1.f / (eye - center).norm();

	if (isRot) {
		fTheta += 0.5f * dt / 1000;
	}
	mat4x4 rotY = Matrix_MakeRotationY(fTheta);
	mat4x4 rotX = Matrix_MakeRotationX(fTheta * 0.5);

	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		vec3 screen_coords[3];
		vec3 world_coords[3];
		for (int j = 0; j < 3; j++) {
			vec3 v = model->vert(face[j]);
			// modle to world
			v = Matrix_MultiplyVector(rotY, v);
			//v = Matrix_MultiplyVector(rotX, v);
			v = v * scale;// Sx
			//v.y -= 1;// Tx
			// world to view
			v = Matrix_MultiplyVector(ModelView, v);
			// Project triangles from 3D --> 2D
			v = Matrix_MultiplyVector(Projection, v);
			// Scale into Screen
			vec3 screenPoint = Matrix_MultiplyVector(ViewPort, v);

			screen_coords[j] = screenPoint;
			world_coords[j] = v;
		}
		// get tri n_vec
		vec3 n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		n.normalize();
		float intensity = n * light_dir;
		if (intensity > 0) {
			vec2 uv[3];
			for (int k = 0; k < 3; k++) {
				uv[k] = model->uv(i, k);
			}
			triangle_tga_texture(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], intensity);
			//device.drawTriangle(screen_coords[0], screen_coords[1], screen_coords[2], 0xffffff);
		}
	}
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
				//diffuse底色/texture
				int u = col.x * 255; // diffuseWidth 
				int v = col.y * 255; // diffuseHeight
				UI32 c = texture[v][u];
				device.setPixel(p.x, p.y, c);
			}
		}
	}
}


void triangle_tga_normal(vec3 t0, vec3 t1, vec3 t2, float ity0, float ity1, float ity2) {
	if (t0.y == t1.y && t0.y == t2.y) return;
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(ity0, ity1); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(ity0, ity2); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(ity1, ity2); }

	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
		vec3 A = t0 + vec3(t2 - t0) * alpha;
		vec3 B = second_half ? t1 + vec3(t2 - t1) * beta : t0 + vec3(t1 - t0) * beta;
		float ityA = ity0 + (ity2 - ity0) * alpha;
		float ityB = second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;

		if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); }

		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (B.x - A.x);
			vec3    P = vec3(A) + vec3(B - A) * phi;
			float ityP = ityA + (ityB - ityA) * phi;
			int idx = P.x + P.y * width;
			if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
			if (P.z > device.zbuffer[(int)P.y][(int)P.x]) {
				device.zbuffer[(int)P.y][(int)P.x] = P.z;
				TGAColor tc = TGAColor(255, 255, 255) * ityP;
				UI32 c = rgb2hex(tc.bgra[2], tc.bgra[1], tc.bgra[0]);
				device.setPixel(P.x, P.y, c);
			}
		}
	}
}

void triangle_tga_texture(vec3 t0, vec3 t1, vec3 t2, vec2 uv0, vec2 uv1, vec2 uv2, float intensity) {
	if (t0.y == t1.y && t0.y == t2.y) return;
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); }

	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

		vec3 A = t0 + (t2 - t0) * alpha;
		vec3 B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;
		vec2 uvA = uv0 + (uv2 - uv0) * alpha;
		vec2 uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;

		if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); }

		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
			vec3   P = (A)+(B - A) * phi;
			vec2 uvP = uvA + (uvB - uvA) * phi;
			int idx = P.x + P.y * width;
			if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
			if (P.z > device.zbuffer[(int)P.y][(int)P.x]) {
				device.zbuffer[(int)P.y][(int)P.x] = P.z;

				TGAColor tc = model->diffuse(uvP);
				tc = tc * intensity;
				UI32 c = rgb2hex(tc.bgra[2], tc.bgra[1], tc.bgra[0]);
				device.setPixel(P.x, P.y, c);
			}
		}
	}
}