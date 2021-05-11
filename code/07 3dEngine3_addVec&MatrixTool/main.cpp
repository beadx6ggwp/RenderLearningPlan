﻿#include "device.h"
#include <chrono> // timer
using namespace std;
using namespace std::chrono;
//-----------------------------------------
void onLoad();
void gameMain();
void update();
void render();
//-----------------------------------------
device_t device;
//-----------------------------------------
typedef struct _Mesh {
	vector<triangle> tris;
	bool LoadFromObjectFile(string sFilename, bool clear = true)
	{
		ifstream f(sFilename);
		if (!f.is_open())
			return false;
		if (clear)tris.clear();
		// Local cache of verts
		vector<vec3> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			strstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				vec3 v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				int f0 = f[0] - 1;
				int f1 = f[1] - 1;
				int f2 = f[2] - 1;
				tris.push_back({ verts[f0], verts[f1], verts[f2] });
			}
		}

		return true;
	}
} mesh;


void fpsCounting();
void MultiplyMatrixVector(mat4x4& m, vec3& i, vec3& o);
uint64_t timeSinceEpochMillisec();
// just test

u32 backcolor = 0xFFFFFF;//0xAARRGGBB;

vec3 v1{ 200,300 }, v2{ 250,400 }, v3{ 100,350 };

mesh meshCube;
int scale = 100;
float fl = 3.5, centZ = 0;
float fTheta = 0;

vec3 vCamera;
mat4x4 matProj;

wchar_t strBuffer[100];
int lastT = (int)timeSinceEpochMillisec(), dt = 0, totalT = 0;;
int fps = 0, frames = 0;

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

	// 順時針
	// Z朝向螢幕, X螢幕指向右邊, Y指向螢幕上面
	meshCube = {
		{
			// SOUTH
			{ 0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f, },
			{ 0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f, },
			// EAST                                                      
			{ 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f, },
			{ 1.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f, 1.0f, },
			// NORTH                                                     
			{ 1.0f, 0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f, },
			{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f, },
			// WEST                                                      
			{ 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f, },
			{ 0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f, },
			// TOP                                                       
			{ 0.0f, 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f, },
			{ 0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f, },
			// BOTTOM                                                    
			{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f, },
			{ 1.0f, 0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f, }
		}
	};
	//meshCube.LoadFromObjectFile("VideoShip.obj");

	// create view project
	matProj = Matrix_MakeProjection(90.0f, (float)device.height / (float)device.width, 0.1f, 1000.0f);

	/*float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFov = 90.0f;
	float fAspectRatio = (float)device.height / (float)device.width;
	float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);
	matProj = {
		fAspectRatio * fFovRad,0,0,0,
		0,fFovRad,0,0,
		0,0,fFar / (fFar - fNear),(-fFar * fNear) / (fFar - fNear),
		0,0,1,0
	};*/
}

void gameMain() {
	fpsCounting();
	swprintf(strBuffer, 100, L"Win32 Render %dx%d, FPS:%4d, dt:%2dms", device.width, device.height, fps, dt);
	SetWindowText(screen_handle, strBuffer);
	update();
	render();
}

void update() {

}
void render() {
	device_clear(&device, 0);

	// -----------
	fillTriangle(&device, v1, v2, v3, 0xFF7700);
	drawTriangle(&device, v1, v2, v3, 0x000000);

	mat4x4 matRotZ, matRotX, matScale;
	fTheta += 0.7f * dt / 1000;

	// Rotation Z
	matRotZ = Matrix_MakeRotationZ(fTheta);

	// Rotation X
	matRotX = Matrix_MakeRotationX(fTheta * 0.5);

	mat4x4 matTrans;
	matTrans = Matrix_MakeTranslation(0.0, 0.0f, 16.0f);

	// from modle to world pos
	// 繞軸旋轉順序 y>x>z or z>y>x
	// here is z>y>x
	mat4x4 matWorld;
	matWorld = Matrix_MakeIdentity(); //init
	matWorld = Matrix_MultiplyMatrix(matRotZ, matWorld); // Transform by rotation
	matWorld = Matrix_MultiplyMatrix(matRotX, matWorld); // Transform by rotation
	matWorld = Matrix_MultiplyMatrix(matTrans, matWorld); // Transform by translation;
	// 先旋轉再平移，由pos開始往外乘，跟巢狀func一樣:I*T*(Rx*(Rz* pos)))

	// Store triagles for rastering later
	vector<triangle> vecTrianglesToRaster;

	// calc tri porjcet
	for (auto tri : meshCube.tris)
	{
		triangle triProjected, triTransformed, triViewed;

		triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
		triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
		triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

		//printf("%f,%f,%f  %f,%f,%f\n", tri.p[0].z, tri.p[1].z, tri.p[2].z, triRotatedZ.p[0].z, triRotatedZ.p[1].z, triRotatedZ.p[2].z);

		//---------------------


		// Use Cross-Product to get surface normal
		vec3 normal, line1, line2;
		line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
		line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
		// line1 cross line2
		normal = Vector_CrossProduct(line1, line2);

		// It's normally normal to normalise the normal
		normal = Vector_Normalise(normal);
		vec3 vCameraRay = Vector_Sub(triTransformed.p[0], vCamera);
		// * unit vector dot is angle
		//if (normal.z < 0)
		if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
		{

			vec3 light_direction = { 0.0f, 0.0f, -1.0f };
			light_direction = Vector_Normalise(light_direction);
			// 光的向量與面向量夾角
			float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

			// Project triangles from 3D --> 2D
			//MultiplyMatrixVector(matProj, triTransformed.p[0], triProjected.p[0]);
			//MultiplyMatrixVector(matProj, triTransformed.p[1], triProjected.p[1]);
			//MultiplyMatrixVector(matProj, triTransformed.p[2], triProjected.p[2]);
			triProjected.p[0] = Matrix_MultiplyVector(matProj, triTransformed.p[0]);
			triProjected.p[1] = Matrix_MultiplyVector(matProj, triTransformed.p[1]);
			triProjected.p[2] = Matrix_MultiplyVector(matProj, triTransformed.p[2]);
			// Scale into view, we moved the normalising into cartesian space
			// out of the matrix.vector function from the previous videos, so
			// do this manually
			triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
			triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
			triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

			// Scale into view
			triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
			triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
			triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
			triProjected.p[0].x *= 0.5f * (float)device.width;
			triProjected.p[0].y *= 0.5f * (float)device.height;
			triProjected.p[1].x *= 0.5f * (float)device.width;
			triProjected.p[1].y *= 0.5f * (float)device.height;
			triProjected.p[2].x *= 0.5f * (float)device.width;
			triProjected.p[2].y *= 0.5f * (float)device.height;


			// Rasterize triangle
			float cc = dp * 255;
			u32 c1 = ((u32)cc) << 16;

			triProjected.col = c1;

			//fillTriangle(&device, triProjected.p[0], triProjected.p[1], triProjected.p[2], c1);
			//drawTriangle(&device, triProjected.p[0], triProjected.p[1], triProjected.p[2], 0);

			// Store triangle for sorting
			vecTrianglesToRaster.push_back(triProjected);
		}
	}

	// Sort triangles from back to front
	// 在使用Z-buffer之前的簡易作法
	sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2)
		 {
			 float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			 float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			 return z1 > z2;
		 });

	for (auto& triProjected : vecTrianglesToRaster)
	{
		// Rasterize triangle
		fillTriangle(&device, triProjected.p[0], triProjected.p[1], triProjected.p[2], triProjected.col);

		//drawTriangle(&device, triProjected.p[0], triProjected.p[1], triProjected.p[2], 0);
	}
}



//------------------------------------------------------------

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