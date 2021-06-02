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
void filltriangle_bery_texture(Vec3f* pts, Vec3f* vt);
void triangle_tga_texture(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, float intensity);
void triangle_tga_normal(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, float ity2);
//--------------------------------------------------------

int* zbuffer = NULL;

Vec3f light_dir(0, 1, 1);// left hand(1,-1,1)
Vec3f eye(0, 0, 3);
Vec3f center(0, 0, 0);
float fTheta = 0;
bool isRot = 1;

Vec3f m2v(Matrix m) {
	return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {
	mat<4, 4, float> m;
	m[0][0] = v.x;
	m[1][0] = v.y;
	m[2][0] = v.z;
	m[3][0] = 1.f;
	return m;
}

Matrix viewport(int x, int y, int w, int h) {
	Matrix m = Matrix::identity();
	m[0][3] = x + w / 2.f;
	m[1][3] = y + h / 2.f;
	m[2][3] = depth / 2.f;

	m[0][0] = w / 2.f;
	m[1][1] = -h / 2.f;
	m[2][2] = depth / 2.f;
	return m;
}
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	Matrix res = Matrix::identity();
	for (int i = 0; i < 3; i++) {
		res[0][i] = x[i];
		res[1][i] = y[i];
		res[2][i] = z[i];
		res[i][3] = -center[i];
	}
	return res;
}

Matrix RotationX(float fAngleRad) {
	Matrix res = Matrix::identity();
	/*
		1,		0,					0,					0,
		0,		cosf(fAngleRad),	-sinf(fAngleRad),	0,
		0,		sinf(fAngleRad),	cosf(fAngleRad),	0,
		0,		0,					0,					1
	*/
	res[1][1] = cosf(fAngleRad);
	res[1][2] = -sinf(fAngleRad);
	res[2][1] = sinf(fAngleRad);
	res[2][2] = cosf(fAngleRad);
	return res;
}
Matrix RotationY(float fAngleRad) {
	Matrix res = Matrix::identity();
	/*
		cosf(fAngleRad),		0,		sinf(fAngleRad),	0,
		0,						1,		0,					0,
		-sinf(fAngleRad),		0,		cosf(fAngleRad),	0,
		0,						0,		0,					1
	*/
	res[0][0] = cosf(fAngleRad);
	res[0][2] = sinf(fAngleRad);
	res[2][0] = -sinf(fAngleRad);
	res[2][2] = cosf(fAngleRad);
	return res;
}
Matrix RotationZ(float fAngleRad) {
	Matrix res = Matrix::identity();
	/*
		cosf(fAngleRad),	-sinf(fAngleRad),		0,			0,
		sinf(fAngleRad),	cosf(fAngleRad),		0,			0,
		0,					0,						1,			0,
		0,					0,						0,			1
	*/
	res[0][0] = cosf(fAngleRad);
	res[0][1] = -sinf(fAngleRad);
	res[1][0] = sinf(fAngleRad);
	res[1][1] = cosf(fAngleRad);
	return res;
}

Matrix RotationByAxis(float x, float y, float z, float theta) {
	Matrix res = Matrix::identity();
	/*
		cosf(fAngleRad),	-sinf(fAngleRad),		0,			0,
		sinf(fAngleRad),	cosf(fAngleRad),		0,			0,
		0,					0,						1,			0,
		0,					0,						0,			1
	*/
	float qsin = sinf(theta * 0.5f);
	float qcos = cosf(theta * 0.5f);
	float w = qcos;
	Vec3f v(x, y, z);
	v.normalize();
	x = v.x * qsin;
	y = v.y * qsin;
	z = v.z * qsin;

	Vec3f v1(1 - 2 * y * y - 2 * z * z, 2 * x * y + 2 * w * z, 2 * x * z - 2 * w * y);
	Vec3f v2(2 * x * y - 2 * w * z, 1 - 2 * x * x - 2 * z * z, 2 * y * z + 2 * w * x);
	Vec3f v3(2 * x * z + 2 * w * y, 2 * y * z - 2 * w * x, 1 - 2 * x * x - 2 * y * y);
	res.set_col(0, embed<4>(v1));
	res.set_col(1, embed<4>(v2));
	res.set_col(2, embed<4>(v3));
	res[0][3] = res[1][3] = res[2][3] = 0;
	res[3][0] = res[3][1] = res[3][2] = 0;
	return res;
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
dice
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
	/*Vec4f A = embed<4>(Vec3f(1, 2, 3));
	mat<4, 1, float> m1;
	m1[0][0] = 2;
	m1[1][0] = 3;
	m1[2][0] = 4;
	m1[3][0] = 1.f;
	Matrix m2 = Matrix::identity();
	Matrix m3 = Matrix::identity();*/

	//A = m2 * m3 * A;	

	/*mat<2, 2, float> m1;
	m1.set_col(0,Vec2f(3, 1));
	m1.set_col(1, Vec2f(1, 2));
	Vec2f v1(-1, 2);
	Vec2f v2 = m1 * v1;*/
	//Vec4f v = embed<4>(Vec3f(1, 2, 3), 2.0f);

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
			 device.width, device.height, fps, deltaT);
	SetWindowText(screen_handle, strBuffer);
	update(deltaT / 1000.0f);
	render();
}

void update(float deltatime) {
	//cout << deltatime << "\n";
	if (isRot)
		fTheta += 0.5f * deltatime;
}

void render() {
	device.clear();

	//device.drawLine(0, 300, 200, 400, 0xff0000);
	Vec3f triTest[] = { Vec3f(0,0,0), Vec3f(80,0,0), Vec3f(0,40,0) };
	device.filltriangle_bery(triTest, 0xff7700);

	// z-buffer穿插測試
	Vec3f triR[] = { Vec3f(50,100,0), Vec3f(200,50,0), Vec3f(200,150,0) };
	device.filltriangle_bery_zbuffer(triR, 0xff0000);
	Vec3f triG[] = { Vec3f(90,40,50), Vec3f(90,160,50), Vec3f(150,100,100) };
	device.filltriangle_bery_zbuffer(triG, 0x00ff00);
	Vec3f triB[] = { Vec3f(100,100,100), Vec3f(180,20,-50), Vec3f(180,180,-50) };
	device.filltriangle_bery_zbuffer(triB, 0x0000ff);

	// clock-wise RGB mapping
	Vec3f cols[] = { Vec3f(1,0,0), Vec3f(0,1,0), Vec3f(0,0,1) };
	Vec3f a(10 + 0, 200 + 0, 0);
	Vec3f tri1[] = { a, Vec3f(a.x + 200,a.y,a.z), Vec3f(a.x + 100,a.y + 150,a.z) };
	device.filltriangle_bery_testRGB(tri1, cols);

	a.y += 200;
	Vec3f cols2[] = { Vec3f(1,1,0), Vec3f(0,1,1), Vec3f(1,0,1) };
	Vec3f tri2[] = { a, Vec3f(a.x + 200,a.y,a.z), Vec3f(a.x + 100,a.y + 150,a.z) };
	device.filltriangle_bery_testRGB(tri2, cols2);

	a.y += 200;
	Vec3f cols3[] = { Vec3f(1,0,0), Vec3f(1,0,0), Vec3f(0,0,1) };
	Vec3f tri3[] = { a, Vec3f(a.x + 200,a.y,a.z), Vec3f(a.x + 100,a.y + 150,a.z) };
	device.filltriangle_bery_testRGB(tri3, cols3);
	//--------------------------------------------------------------------------------------------------

	Vec3f p1(300, 300, 0), size(200, 200, 0);
	Vec3f vt1[] = { Vec3f(0, 0, 0), Vec3f(1, 0, 0), Vec3f(0, 1, 0) };
	Vec3f triT1[] = { p1, Vec3f(p1.x + size.x,p1.y,0), Vec3f(p1.x,p1.y + size.y,0) };
	filltriangle_bery_texture(triT1, vt1);

	p1.x += 2; p1.y += 2;
	Vec3f vt2[] = { Vec3f(1, 0, 0), Vec3f(1, 1, 0), Vec3f(0, 1, 0) };
	Vec3f triT2[] = { Vec3f(p1.x + size.x,p1.y,0), Vec3f(p1.x + size.x,p1.y + size.y,0), Vec3f(p1.x,p1.y + size.y,0) };
	filltriangle_bery_texture(triT2, vt2);

	// ------
	//Matrix ModelTrans = RotationY(fTheta);

	//                                 x, y, z, angle
	//Matrix ModelTrans = RotationByAxis(1, 0, 0, fTheta);
	//Matrix ModelTrans = RotationByAxis(0, 1, 0, fTheta);
	//Matrix ModelTrans = RotationByAxis(0, 0, 1, fTheta);
	//Matrix ModelTrans = RotationByAxis(1, 1, 0, fTheta);
	Matrix ModelTrans = RotationByAxis(1, 1, 1, fTheta);

	Matrix ViewCamera = lookat(eye, center, Vec3f(0, 1, 0));
	Matrix Projection = Matrix::identity();
	Matrix ViewPort = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	Projection[3][2] = -1.f / (eye - center).norm();


	for (int i = 0; i < model->nfaces(); i++) {
		std::vector<int> face = model->face(i);
		Vec3i screen_coords[3] = {};
		Vec3f world_coords[3] = {};
		float intensity[3] = {};
		for (int j = 0; j < 3; j++) {
			Vec3f v = model->vert(face[j]);

			// world_coords沒有正確被轉換，還是原本模型的座標
			//Vec4f scp = ViewPort * Projection * ViewCamera * ModelTrans * embed<4>((v * scale));
			//screen_coords[j] = Vec3f(scp[0] / scp[3], scp[1] / scp[3], scp[2] / scp[3]);
			//world_coords[j] = v;

			Vec4f mvp = Projection * ViewCamera * ModelTrans * embed<4>((v * scale));
			Vec4f scp = ViewPort * mvp;
			screen_coords[j] = Vec3i(scp[0] / scp[3], scp[1] / scp[3], scp[2] / scp[3]);
			world_coords[j] = Vec3f(mvp[0], mvp[1], mvp[2]);

			// relative light: if light on face, then always be light
			//intensity[j] = model->norm(i, j) * light_dir;

			// fixed light: static light source  
			Vec4f rotNormal = ModelTrans * embed<4>(model->norm(i, j));
			intensity[j] = Vec3f(rotNormal[0] / rotNormal[3], rotNormal[1] / rotNormal[3], rotNormal[2] / rotNormal[3]) * light_dir;

		}
		triangle_tga_normal(screen_coords[0], screen_coords[1], screen_coords[2], intensity[0], intensity[1], intensity[2]);
		//device.drawTriangle(screen_coords[0], screen_coords[1], screen_coords[2], 0xffff00);

	}
}


void filltriangle_bery_texture(Vec3f* pts, Vec3f* vt) {
	Vec2f bboxmin((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
	Vec2f bboxmax(-(std::numeric_limits<float>::max)(), -(std::numeric_limits<float>::max)());
	Vec2f clamp(width - 1, height - 1);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = (std::max)(0.f, (std::min)(bboxmin[j], pts[i][j]));
			bboxmax[j] = (std::min)(clamp[j], (std::max)(bboxmax[j], pts[i][j]));
		}
	}

	Vec3f p, bc_screen;
	for (p.x = bboxmin.x; p.x <= bboxmax.x; p.x++) {
		for (p.y = bboxmin.y; p.y <= bboxmax.y; p.y++) {
			bc_screen = barycentric(proj<2>(pts[0]), proj<2>(pts[1]), proj<2>(pts[2]), proj<2>(p));
			// check point in tri
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

			p.z = 0;
			for (int i = 0; i < 3; i++) p.z += pts[i][2] * bc_screen[i];
			if (p.z > device.zbuffer[(int)p.y][(int)p.x]) {
				device.zbuffer[(int)p.y][(int)p.x] = p.z;

				Vec3f col(
					bc_screen.x * vt[0].x + bc_screen.y * vt[1].x + bc_screen.z * vt[2].x,
					bc_screen.x * vt[0].y + bc_screen.y * vt[1].y + bc_screen.z * vt[2].y,
					bc_screen.x * vt[0].z + bc_screen.y * vt[1].z + bc_screen.z * vt[2].z
				);
				//diffuse底色/texture
				int u = col.x * 255; // diffuseWidth 
				int v = col.y * 255; // diffuseHeight
				UI32 c = texture[v][u];
				device.setPixel(p.x, p.y, c);
			}
		}
	}

}

// Gouraud shading
void triangle_tga_normal(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, float ity2) {
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(ity0, ity1); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(ity0, ity2); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(ity1, ity2); }

	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here

		Vec3i A = t0 + Vec3i(Vec3f(t2 - t0) * alpha);
		Vec3i B = second_half ? t1 + Vec3i(Vec3f(t2 - t1) * beta) : t0 + Vec3i(Vec3f(t1 - t0) * beta);

		float ityA = ity0 + (ity2 - ity0) * alpha;
		float ityB = second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;

		if (A.x > B.x) { std::swap(A, B); std::swap(ityA, ityB); }
		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (B.x - A.x);
			Vec3i    P = Vec3f(A) + Vec3f(B - A) * phi;
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
void triangle_tga_texture(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, float intensity) {
	if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
	if (t0.y > t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); }
	if (t0.y > t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); }
	if (t1.y > t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); }

	int total_height = t2.y - t0.y;
	for (int i = 0; i < total_height; i++) {
		bool second_half = i > t1.y - t0.y || t1.y == t0.y;
		int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
		float alpha = (float)i / total_height;
		float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height; // be careful: with above conditions no division by zero here

		Vec3i A = t0 + Vec3i(Vec3f(t2 - t0) * alpha);
		Vec3i B = second_half ? t1 + Vec3i(Vec3f(t2 - t1) * beta) : t0 + Vec3i(Vec3f(t1 - t0) * beta);
		Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
		Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;

		if (A.x > B.x) { std::swap(A, B); std::swap(uvA, uvB); }

		for (int j = A.x; j <= B.x; j++) {
			float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
			Vec3i   P = Vec3f(A) + Vec3f(B - A) * phi;
			Vec2i uvP = uvA + (uvB - uvA) * phi;
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