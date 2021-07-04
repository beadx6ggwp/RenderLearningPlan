#include <iostream>
#include <cmath>
#include <limits>

#include "tool.h"
#include "geometry.h"
#include "tgaimage.h"

#include "screen.h"
#include "device.h"
#include "model.h"
#include "my_gl.h"

using namespace std;
//-----------------------------------------
void onLoad();
void gameMain();
void update(float deltatime);
void render();
//-----------------------------------------
const int width = 800, height = 800, depth = 255;
Device device;

UI32 texture[256][256];
void filltriangle_bery_texture(Vec3f* pts, Vec3f* vt);
void triangle_tga_texture(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, float intensity);
void triangle_tga_normal(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, float ity2);
//--------------------------------------------------------
// world space is right-hand(up is y, right is x, z facing me)

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
//-----------------------------------------------------------

void testCase();
//--------------------------------------------------------

Model* model = NULL;
Model* model_obj1 = NULL;
Model* model_obj2 = NULL;
string dirPath = "../_objfile/testTexture/";
/*
african_head
rock
floor
diablo3_pose
wall1
capsule
dice
*/

float tx = 0, ty = 0, tz = 0;
Vec3f light_dir(1, 1, 1);
Vec3f       eye(0, 0, 0);//1, 1, 3
Vec3f    center(-1, 0, 0);
Vec3f        up(0, 1, 0);

Vec3f	 vCamera(0, 0, 0);
Vec3f   vLookDir(0, 0, 1);
float fYaw = 0;


struct GouraudShader : public IShader {
	Vec3f varying_intensity; // written by vertex shader, read by fragment shader
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;
	mat<4, 4, float> uniform_MIT;

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_intensity[nthvert] = (std::max)(0.f, model->normal(iface, nthvert) * light_dir); // get diffuse lighting intensity
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
		return Viewport * Projection * ModelView * gl_Vertex; // transform => screen coordinates
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		float intensity = varying_intensity * bar;
		float r, g, b;
		r = 255 * intensity;
		g = 255 * intensity;
		b = 255 * intensity;
		color = rgb2hex(r, g, b);
		return false;
	}
};

struct GouraudShader2 : public IShader {
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;
	mat<4, 4, float> uniform_MIT;
	float scale = 0.3;

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);
		gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
		// 計算此三角形與燈光的夾角，決定亮度
		varying_intensity[nthvert] = (std::max)(0.f, model->normal(iface, nthvert) * light_dir);
		return gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		// 對三個頂點的亮度根據uv插值，決定此點的插植亮度為何
		float intensity = varying_intensity * bar;
		float r, g, b;

		// color intensities [1, .85, .60, .45, .30, .15, 0]
		if (intensity > .85) intensity = 1;
		else if (intensity > .60) intensity = .80;
		else if (intensity > .45) intensity = .60;
		else if (intensity > .30) intensity = .45;
		else if (intensity > .15) intensity = .30;
		else intensity = 0;

		// orange
		r = 255 * intensity;
		g = 125 * intensity;
		b = 0 * intensity;
		color = rgb2hex(r, g, b);
		return false;
	}
};

struct TextureShader : public IShader {
	Vec3f varying_intensity; // written by vertex shader, read by fragment shader
	mat<2, 3, float> varying_uv;  // same as above
	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	float scale = 0.3;

	virtual Vec4f vertex(int iface, int nthvert) {

		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_intensity[nthvert] = (std::max)(0.f, model->normal(iface, nthvert) * light_dir);
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		float intensity = varying_intensity * bar;   // interpolate intensity for the current pixel
		Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
		TGAColor tc = model->diffuse(uv) * intensity;
		color = rgb2hex(tc[2], tc[1], tc[0]);//bgra
		return false;
	}
};

struct NornmalImgShader : public IShader {
	// bug
	Vec3f varying_intensity;
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;
	mat<4, 4, float> uniform_MIT;
	float scale = 0.3;

	virtual Vec4f vertex(int iface, int nthvert) {

		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_intensity[nthvert] = (std::max)(0.f, model->normal(iface, nthvert) * light_dir);
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		float intensity = varying_intensity * bar;
		Vec2f uv = varying_uv * bar;

		TGAColor tc = model->normalmap_.get(uv[0] * model->normalmap_.get_width(), uv[1] * model->normalmap_.get_height()) * intensity;
		color = rgb2hex(tc[2], tc[1], tc[0]);//bgra
		return false;
	}
};

struct NormalShader : public IShader {
	/*
	|v0x v1x v2x|
	|v0y v1y v2y|
	*/
	mat<2, 3, float> varying_uv;
	mat<4, 4, float> uniform_M;
	mat<4, 4, float> uniform_MIT;
	float scale = 0.3;

	virtual Vec4f vertex(int iface, int nthvert) {
		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);
		return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		Vec2f uv = varying_uv * bar;
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();

		float intensity = (std::max)(0.1f, n * l);
		TGAColor tc = model->diffuse(uv) * intensity;
		color = rgb2hex(tc[2], tc[1], tc[0]);//bgra
		return false;
	}
};

//90.0f, (float)device.height / (float)device.width, 0.1f, 1000.0f
Matrix makeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	Matrix matrix = Matrix::identity();
	Vec3f c0(fAspectRatio * fFovRad, 0, 0);
	Vec3f c1(0, fFovRad, 0);
	Vec3f c2(0, 0, fFar / (fFar - fNear));
	Vec3f c3(0, 0, (fFar * fNear) / (fFar - fNear));
	matrix.set_col(0, embed<4>(c0, 0.0f));
	matrix.set_col(1, embed<4>(c1, 0.0f));
	matrix.set_col(2, embed<4>(c2, 1.0f));
	matrix.set_col(3, embed<4>(c3, 0.0f));
	return matrix;
}

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
	//model = new Model(dirPath + objName);

	model_obj1 = new Model(dirPath + "floor.obj");
	model_obj2 = new Model(dirPath + "african_head.obj");
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

	//eye.x = 10 * sin(timeSinceEpochMillisec() * 0.001);
	//eye.z = 10 * sin(timeSinceEpochMillisec() * 0.001) + 10.5;

	//vCamera.x = 1 * sin(timeSinceEpochMillisec() * 0.001);
	//vCamera.y = 1 * sin(timeSinceEpochMillisec() * 0.001);

	//fYaw += 0.5f * deltatime;

	int sp = 2;
	if (screen_keys['R']) {
		vCamera = Vec3f(0, 0, 0);
		fYaw = 0;
	}
	if (screen_keys[VK_UP]) {
		vCamera.y += sp * deltatime;
	}
	if (screen_keys[VK_DOWN]) {
		vCamera.y -= sp * deltatime;
	}
	if (screen_keys[VK_RIGHT]) {
		// get vLookDir right normal vec
		Vec3f vRight(vLookDir.z, 0, -vLookDir.x);
		Vec3f newMove = vRight * sp * deltatime;
		vCamera = vCamera + newMove;

	}
	if (screen_keys[VK_LEFT]) {
		// get vLookDir left normal vec
		Vec3f vLeft(-vLookDir.z, 0, vLookDir.x);
		Vec3f newMove = vLeft * sp * deltatime;
		vCamera = vCamera + newMove;
	}

	Vec3f vForward = vLookDir * 5.0 * deltatime;

	if (screen_keys['W']) {
		vCamera = vCamera - vForward;
	}
	if (screen_keys['S']) {
		vCamera = (vCamera + vForward);
	}
	if (screen_keys['A']) {
		fYaw -= 1.0f * 0.7f * deltatime;
	}
	if (screen_keys['D']) {
		fYaw += 1.0f * 0.7f * deltatime;
	}

	cout << vCamera << "| " << vLookDir << "| " << fYaw * 180.0f / 3.14f << "\n";
}

void render() {
	device.clear();

	// ------
	testCase();
	// ------

	Vec3f vTarget(0, 0, 1);
	Matrix matCameraRotY = RotationY(fYaw);

	Vec4f temp = matCameraRotY * embed<4>(vTarget);
	vLookDir.x = temp[0], vLookDir.y = temp[1], vLookDir.z = temp[2];

	vTarget = vCamera + vLookDir;

	lookat(vTarget, vCamera, up);
	viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	projection(-1.f / (vTarget - vCamera).norm());

	//lookat(eye, center, up);
	//viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	//projection(-1.f / (eye - center).norm());
	//Projection = makeProjection(90.0f, ((float)height / (float)width)*0.75, 0.1f, 1000.0f);
	light_dir.normalize();

	Matrix ModelTrans = RotationByAxis(1, 1, 1, fTheta);

	TextureShader shader;

	// set tran
	shader.uniform_M = Projection * ModelView;
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();

	model = model_obj1;
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++) {
			screen_coords[j] = shader.vertex(i, j);
		}
		triangle(screen_coords, shader, device);
	}
	model = model_obj2;
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		for (int j = 0; j < 3; j++) {
			screen_coords[j] = shader.vertex(i, j);
		}
		triangle(screen_coords, shader, device);
	}
}

void testCase() {
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