#include <iostream>
#include <cmath>
#include <limits>
#include <map>
#include <iomanip>

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
const int width = 400, height = 400, depth = 255;
Device device;

UI32 texture[256][256];
void filltriangle_bery_texture(Vec3f* pts, Vec3f* vt);
void triangle_tga_texture(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, float intensity);
void triangle_tga_normal(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, float ity2);
//--------------------------------------------------------
// world space is right-hand(up is y, right is x, z facing me)

float fTheta = 0;
bool isRot = 1;
//-----------------------------------------------------------

void testCase();
//--------------------------------------------------------

Model* model = NULL;

Model* model_test = NULL;
Model* model_head = NULL;
Model* model_cube = NULL;
Model* model_rock = NULL;
Model* model_floor = NULL;

string dirPath = "../_objfile/";

map<string, Model*> model_list;
vector<string> render_list;
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
Vec3f light_dir(0, 0.5, 1);

Vec3f       eye(0, 0, 0);//1, 1, 3
Vec3f    center(-1, 0, 0);
Vec3f        up(0, 1, 0);

Vec3f	 vCamera(0, 0, 2);// (0,0,0)
Vec3f   vLookDir(0, 0, 1);//(0, 0, 1)
float fYaw = 0.0;

Matrix ModelTrans_ = Matrix::identity();

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
		//gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
		// 計算此三角形與燈光的夾角，決定亮度
		varying_intensity[nthvert] = (std::max)(0.f, model->normal(iface, nthvert) * light_dir);

		return Projection * ModelView * ModelTrans_ * gl_Vertex;
		//return Viewport * Projection * ModelView * gl_Vertex;
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
	float scale = 0.5;

	virtual Vec4f vertex(int iface, int nthvert) {

		varying_uv.set_col(nthvert, model->uv(iface, nthvert));
		varying_intensity[nthvert] = (std::max)(0.f, model->normal(iface, nthvert) * light_dir);
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);

		Vec4f pos = Projection * ModelView * ModelTrans_ * gl_Vertex;
		return pos;
		//return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		float intensity = varying_intensity * bar;   // interpolate intensity for the current pixel
		Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
		TGAColor tc = model->diffuse(uv) * 1;// fixed light: model->diffuse(uv) * intensity
		color = rgb2hex(tc[2], tc[1], tc[0]);//bgra
		return false;
	}
};
struct LineShader : public IShader {
	Vec3f varying_intensity; // written by vertex shader, read by fragment shader
	mat<2, 3, float> varying_uv;  // same as above
	mat<4, 4, float> uniform_M;   //  Projection*ModelView
	mat<4, 4, float> uniform_MIT; // (Projection*ModelView).invert_transpose()
	float scale = 0.5;
	float range = 1e-2;//1e-2

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);
		return Viewport * Projection * ModelView * ModelTrans_ * gl_Vertex;
	}
	virtual bool fragment(Vec3f bar, UI32& color) {
		if (bar[0] < range || bar[1] < range || bar[2] < range) {
			color = rgb2hex(0 * 255, 0 * 255, 0 * 255);
		}
		else {
			color = rgb2hex(255, 255, 255);
			//return true;
		}
		// 或是return true表示這個點不渲染，即可表示純線框
		return false;
	}
};
struct WireframeShader : public IShader {
	/// wireframe-display-with-barycentric-coordinates
	// Implement: Two Methods for Antialiased Wireframe Drawing with Hidden Line Removal

	/*
	color = Vec3f(bar[0], bar[1], bar[2]); 這行等價於下面，因為只插值(1,0,0),(0,1,0),(0,0,1)的話，三行合在一起也是一樣的
	如果不是(1,0,0),(0,1,0),(0,0,1)就必須寫完整了
	vec3 cols[3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
	vec3 col = {
				bc_screen.x * colors[0].x + bc_screen.y * colors[1].x + bc_screen.z * colors[2].x,
				bc_screen.x * colors[0].y + bc_screen.y * colors[1].y + bc_screen.z * colors[2].y,
				bc_screen.x * colors[0].z + bc_screen.y * colors[1].z + bc_screen.z * colors[2].z
			};
	*/
	mat<4, 4, float> uniform_M;
	mat<4, 4, float> uniform_MIT;
	float scale = 0.5;

	Vec2f screen_coords[3];

	//Vec3f wire_color = Vec3f(bar[0], bar[1], bar[2]);
	Vec3f wire_color = Vec3f(0, 0, 0);
	Vec3f fill_color = Vec3f(1, 1, 1);
	float width = 5; // 線框寬度
	float feather = 0.5; // 反鋸齒作用的範圍+/-
	int mode = 1;

	virtual Vec4f vertex(int iface, int nthvert) {
		Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert) * scale);
		Vec4f pos = Viewport * Projection * ModelView * ModelTrans_ * gl_Vertex;

		//pos = Viewport * Projection * ModelTrans_ * gl_Vertex;

		// 紀錄這片三角形投影後的三個頂點, Clip Space /w-> NDC ->Screen
		screen_coords[nthvert] = proj<2>(pos / pos[3]);
		return pos;
	}
	virtual bool fragment(Vec3f bar, UI32& color) {

		float d = getMinDistToEdge(bar);

		//wire_color = Vec3f(bar[0], bar[1], bar[2]);

		bool discard = false;
		if (mode == 1) {
			//wire_color = Vec3f(bar[0], bar[1], bar[2]);
			discard = drawLine(d, bar, color);
		}
		else {
			discard = drawDash(d, bar, color);
		}


		return discard;
	}
	bool drawDash(float d, Vec3f bar, UI32& color) {
		// draw dash
		// ref:https://web.archive.org/web/20150906185520/https://forum.libcinder.org/topic/wireframe-shader-implementation


		float positionAlong = (std::max)(bar.x, bar.y);
		if (bar.y < bar.x && bar.y < bar.z) {
			positionAlong = 1.0 - positionAlong;
		}

		// one way to calculate interpolation factor
		float f = bar.x;
		if (bar.x < min(bar.y, bar.z))
			f = bar.y;
		float PI = 3.14159265;
		float stipple = pow(clamp(2 * sin(f * 21 * PI), 0, 1), 4);
		float thickness = 1 * stipple;// change width

		Vec3f c;
		float I = smoothstep(thickness - feather, thickness + feather, d);

		/*if (stipple > 0) {
			c = wire_color * I + fill_color * (1.0 - I);
		}
		else
		{
			c = fill_color;
		}*/

		if (d < thickness) {
			//float I = exp2(-2 * d * d);
			c = wire_color * I + fill_color * (1.0 - I);
			//c = wire_color;
		}
		else {
			c = fill_color;
			//return true; //discord背景像素的話就可以繪製純線框
		}
		color = rgb2hex(c[0] * 255, c[1] * 255, c[2] * 255);

		return false; //no discard this pixel
	}
	bool drawLine(float d, Vec3f bar, UI32& color) {
		//float I = exp2(-2 * d * d);
		float I = smoothstep(width - feather, width + feather, d);// width +/- 1
		//fill_color = Vec3f(0, 0, 0);
		Vec3f c;

		//沒有反鋸齒
	   /*if (d < width) {
		   c = wire_color;
	   }
	   else {
		   c = fill_color;
	   }*/
	   // 線框顏色插值RGB
	   //wire_color = Vec3f(bar[0], bar[1], bar[2]);
		c = wire_color * I + fill_color * (1.0 - I);
		color = rgb2hex(c[0] * 255, c[1] * 255, c[2] * 255);

		// 僅繪製線框的方法
		//if (d > 1.25) { return true; }
		//if (I <= 0.25) { return true; }
		return false;
	}
	float smoothstep(float edge0, float edge1, float x) {
		// https://stackoverflow.com/questions/28889210/smoothstep-function
		// https://en.wikipedia.org/wiki/Smoothstep
		// https://thebookofshaders.com/glossary/?search=smoothstep
		x = clamp((x - edge0) / (edge1 - edge0), 0.0, 2.0);
		return  exp2(-2 * x * x);
	}
	float clamp(double d, double min, double max) {
		const double t = d < min ? min : d;
		return t > max ? max : t;
	}
	float getMinDistToEdge(Vec3f bar) {
		float dist[3];
		auto getLength = [&](Vec2f& v) {	return std::sqrt(v.x * v.x + v.y * v.y); };
		// 透過三個頂點與重心座標回推掃描線p點，proj<2>:將Vec3捨棄z值轉換為Vec2
		// P = A*alpha + B*beta + C*gamma
		Vec2f p = proj<2>(screen_coords[0] * bar[0] +
						  screen_coords[1] * bar[1] +
						  screen_coords[2] * bar[2]);

		Vec2f v0 = screen_coords[2] - screen_coords[1];
		Vec2f v1 = screen_coords[2] - screen_coords[0];
		Vec2f v2 = screen_coords[1] - screen_coords[0];
		// v1,v2外積取得三角形面積，原本要在乘上的1/2，不過後續的運算會消掉
		float area = abs(v1.x * v2.y - v1.y * v2.x);

		// area*alpha:因為重心座標即為為a,b,c三區域佔總體的面積比值
		// 可以直接將整體面積(Area)乘上a,b,c重心座標來取得各塊區域面積
		// 再透過三角形面積=底x高*1/2的公式，與三塊面積取高，得出p點與最各邊界的高，也就是距離d為何
		dist[0] = area * bar[0] / getLength(v0);
		dist[1] = area * bar[1] / getLength(v1);
		dist[2] = area * bar[2] / getLength(v2);

		// 找出最近的邊界距離
		return min(dist[0], min(dist[1], dist[2]));
	}
};

struct NormalImgShader : public IShader {
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

		return Viewport * Projection * ModelView * ModelTrans_ * gl_Vertex;
		//return Viewport * Projection * ModelView * gl_Vertex;
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
		return Viewport * Projection * ModelView * ModelTrans_ * gl_Vertex;
		//return Viewport * Projection * ModelView * gl_Vertex;
	}

	virtual bool fragment(Vec3f bar, UI32& color) {
		Vec2f uv = varying_uv * bar;// 重心座標插植
		// 現在的座標是經過MVP後的光柵階段，所要得出正確的法向量反射，需要反推回空間中的方向
		Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
		Vec3f l = proj<3>(uniform_M * embed<4>(light_dir)).normalize();// 光源方向相對於攝影機移動

		float intensity = (std::max)(0.1f, n * l);
		TGAColor tc = model->diffuse(uv) * intensity;
		color = rgb2hex(tc[2], tc[1], tc[0]);//bgra
		return false;
	}
};

Matrix makePointAt(Vec3f& pos, Vec3f& target, Vec3f& up) {
	Matrix matrix = Matrix::identity();
	Vec3f newForward = (target - pos).normalize();

	Vec3f a = newForward * (up * newForward);
	Vec3f newUp = up - a;
	newUp = newUp.normalize();

	Vec3f newRight = cross(newUp, newForward);

	Vec3f c0(newRight.x, newRight.y, newRight.z);
	Vec3f c1(newUp.x, newUp.y, newUp.z);
	Vec3f c2(newForward.x, newForward.y, newForward.z);
	Vec3f c3(pos.x, pos.y, pos.z);
	matrix.set_col(0, embed<4>(c0, 0.0f));
	matrix.set_col(1, embed<4>(c1, 0.0f));
	matrix.set_col(2, embed<4>(c2, 0.0f));
	matrix.set_col(3, embed<4>(c3, 1.0f));
	return matrix;
}
Matrix QuickInverse(Matrix& mat) {
	Matrix matrix = Matrix::identity();

	Vec3f c0(mat[0][0], mat[0][1], mat[0][2]);
	Vec3f c1(mat[1][0], mat[1][1], mat[1][2]);
	Vec3f c2(mat[2][0], mat[2][1], mat[2][2]);
	matrix.set_col(0, embed<4>(c0, 0.0f));
	matrix.set_col(1, embed<4>(c1, 0.0f));
	matrix.set_col(2, embed<4>(c2, 0.0f));
	matrix[0][3] = -(matrix[0][0] * mat[0][3] + matrix[0][1] * mat[1][3] + matrix[0][2] * mat[2][3]);
	matrix[1][3] = -(matrix[1][0] * mat[0][3] + matrix[1][1] * mat[1][3] + matrix[1][2] * mat[2][3]);
	matrix[2][3] = -(matrix[2][0] * mat[0][3] + matrix[2][1] * mat[1][3] + matrix[2][2] * mat[2][3]);

	return matrix;
}

//90.0f, (float)device.height / (float)device.width, 0.1f, 1000.0f
Matrix makeProjection(float fovDeg, float aspect, float zNear, float zFar)
{
	gl_zNear = zNear;
	gl_zFar = zFar;

	float tanHalfFovy = tanf(fovDeg * 0.5f / 180.0f * 3.14159f);
	Matrix matrix = Matrix::identity();
	Vec3f c0(1.0f / (tanHalfFovy * aspect), 0, 0);
	Vec3f c1(0, 1.0f / (tanHalfFovy), 0);
	Vec3f c2(0, 0, -(zFar + zNear) / (zFar - zNear));
	Vec3f c3(0, 0, -2 * (zFar * zNear) / (zFar - zNear));
	matrix.set_col(0, embed<4>(c0, 0.0f));
	matrix.set_col(1, embed<4>(c1, 0.0f));
	matrix.set_col(2, embed<4>(c2, -1.0f));
	matrix.set_col(3, embed<4>(c3, 0.0f));

	// matrix = M(ortho)*M(persp)
	return matrix;
}

void RenderModel(string modelName, IShader& shader) {
	model = model_list[modelName];
	cout << "-\t-\t-\t-\t-\t-\n";
	for (int i = 0; i < model->nfaces(); i++) {
		Vec4f screen_coords[3];
		cout << "[" << i << "]";
		for (int j = 0; j < 3; j++) {
			// 最後再處理viewport
			screen_coords[j] = shader.vertex(i, j);

			//cout << fixed << setprecision(2) << t << " ";
			screen_coords[j] = Viewport * screen_coords[j];

			auto t = screen_coords[j] / screen_coords[j][3];
			float a = (gl_zFar - gl_zNear) / 2;
			float b = (gl_zFar + gl_zNear) / 2;
			//(t[2] - b) / a			
			cout << "\t" << fixed << setprecision(2) << t[2] << "," << screen_coords[j][3] << " ";
		}
		cout << "\n";
		triangle2(screen_coords, shader, device);
	}
}

int main(void) {

	const TCHAR* title = _T("Win32");

	if (screen_init(width, height, title))
		return -1;

	device.init(width, height, screen_fb);
	device.background = rgb2hex(0.85 * 255, 0.85 * 255, 1 * 255);


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
	Matrix testmat = Matrix::identity();
	testmat.set_col(0, embed<4>(Vec3f(1, 2, 3), 1.f));
	cout << testmat;

	// 生成藍白方格的texture
	int i, j;
	for (j = 0; j < 256; j++) {
		for (i = 0; i < 256; i++) {
			int x = i / 32, y = j / 32;
			texture[j][i] = ((x + y) & 1) ? 0xffffff : 0x3fbcef;
		}
	}
	//model = new Model(dirPath + objName);

	model_cube = new Model(dirPath + "testTexture/dice.obj");
	model_rock = new Model(dirPath + "testTexture/rock.obj");
	model_floor = new Model(dirPath + "testTexture/floor.obj");

	//model_test = new Model("../_objfile/test_tri1.obj");// ../_objfile/teapot.obj
	model_test = new Model("../_objfile/Marry.obj");// ../_objfile/teapot.obj

	model_list["floor"] = new Model(dirPath + "testTexture/floor.obj");
	model_list["rock"] = new Model(dirPath + "testTexture/rock.obj");
	model_list["Marry"] = new Model("../_objfile/Marry.obj");
	model_list["african_head"] = new Model(dirPath + "african/african_head.obj");
	model_list["african_head_eye_inner"] = new Model(dirPath + "african/african_head_eye_inner.obj");
	model_list["african_head_eye_outer"] = new Model(dirPath + "african/african_head_eye_outer.obj");
	model_list["matsumoto"] = new Model(dirPath + "testTexture/matsumoto.obj");
	model_list["capsule"] = new Model(dirPath + "testTexture/capsule.obj");
	model_list["test_tri1"] = new Model(dirPath + "test_tri1.obj");
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

	float sp = 0.5;
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

	Vec3f vForward = vLookDir * sp * deltatime;

	if (screen_keys['W']) {
		vCamera = vCamera - vForward;
	}
	if (screen_keys['S']) {
		vCamera = (vCamera + vForward);
	}
	if (screen_keys['A']) {
		fYaw += 1.0f * 0.7f * deltatime;
	}
	if (screen_keys['D']) {
		fYaw -= 1.0f * 0.7f * deltatime;
	}

	//cout << vCamera << "| " << vLookDir << "| " << fYaw * 180.0f / 3.14f << "\n";
}

void render() {
	device.clear();

	// ------
	//testCase();
	// ------

	Vec3f vTarget(0, 0, 1), vUp(0, 1, 0);
	Matrix matCameraRotY = RotationY(fYaw);

	vLookDir = embed<3>(matCameraRotY * embed<4>(vTarget));

	vTarget = vCamera + vLookDir;

	//lookat(vTarget, vCamera, vUp);
	Matrix matCamera = makePointAt(vCamera, vTarget, vUp);
	Matrix matView = QuickInverse(matCamera);
	ModelView = matView;
	//cout << ModelView << '\n';

	//projection(-1.f / (vTarget - vCamera).norm());
	Projection = makeProjection(90.0f, (float)width / (float)height, 0.1f, 1000.0f);

	//viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	viewport(0, 0, width, height);


	light_dir.normalize();

	// set tran
	Matrix ModelTrans = RotationByAxis(1, 1, 1, fTheta);

	// texture shader
	// Obj1----------------------------
	//device.background = 0;



	TextureShader shader;
	ModelTrans_ = Matrix::identity();
	shader.uniform_M = Projection * ModelView;
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();
	RENDER_MODE = 2;
	RenderModel("floor", shader);

	return;


	WireframeShader testShader;
	ModelTrans_ = Matrix::identity();
	ModelTrans_.set_col(3, embed<4>(Vec3f(0.0, 0, 0)));
	testShader.uniform_M = Projection * ModelView;
	testShader.uniform_MIT = (Projection * ModelView).invert_transpose();

	RenderModel("test_tri1", testShader);


	ModelTrans_ = Matrix::identity();
	ModelTrans_.set_col(3, embed<4>(Vec3f(1.0, 0, 0)));
	shader.uniform_M = Projection * ModelView;
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();


	RenderModel("floor", shader);

	ModelTrans_ = Matrix::identity();
	ModelTrans_.set_col(3, embed<4>(Vec3f(-1.0, 0, 0)));
	shader.uniform_M = Projection * ModelView;
	shader.uniform_MIT = (Projection * ModelView).invert_transpose();

	RenderModel("floor", shader);

	// ----------------------------
	// Obj2----------------------------
	NormalImgShader shaderHead_nm;
	shaderHead_nm.scale = 0.4;
	ModelTrans_ = Matrix::identity();
	ModelTrans_.set_col(3, embed<4>(Vec3f(-0.3, 0, -0.4)));
	shaderHead_nm.uniform_M = Projection * ModelView;
	shaderHead_nm.uniform_MIT = (Projection * ModelView).invert_transpose();

	RenderModel("african_head", shaderHead_nm);

	// Obj3----------------------------
	TextureShader shaderHead_test;
	shaderHead_test.scale = 0.4;
	ModelTrans_ = Matrix::identity();
	ModelTrans_.set_col(3, embed<4>(Vec3f(0.3, 0, 0.4)));
	shaderHead_test.uniform_M = Projection * ModelView;
	shaderHead_test.uniform_MIT = (Projection * ModelView).invert_transpose();

	render_list = { "african_head","african_head_eye_inner" };
	for (auto name : render_list) {
		RenderModel(name, shaderHead_test);
	}

	// rock----------------------------
	//device.background = 0;
	TextureShader shader_rock;
	ModelTrans_ = Matrix::identity();

	ModelTrans_ = RotationByAxis(1, 0, 0, fTheta * 1);
	ModelTrans_ = RotationByAxis(0, 1, 0, fTheta * 1.5) * ModelTrans_;
	ModelTrans_.set_col(3, embed<4>(Vec3f(-1.0, -0.2, 0)));// right-hand
	shader_rock.scale = 0.3;
	shader_rock.uniform_M = Projection * ModelView;
	shader_rock.uniform_MIT = (Projection * ModelView).invert_transpose();


	RenderModel("capsule", shader_rock);

	// ----------------
	//ModelTrans_ = RotationByAxis(1, 0, 0, fTheta * 1);
	//ModelTrans_ = RotationByAxis(0, 1, 0, fTheta * 1.5) * ModelTrans_;
	ModelTrans_ = Matrix::identity();
	ModelTrans_.set_col(3, embed<4>(Vec3f(1.0, -0.4, 0)));// right-hand
	GouraudShader2 lineshader;
	lineshader.scale = 0.4;
	//lineshader.width = 0.1;
	//lineshader.mode = 2;
	RenderModel("Marry", lineshader);
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