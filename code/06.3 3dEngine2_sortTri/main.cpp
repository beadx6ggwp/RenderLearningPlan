#include "device.h"
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
			{ 0.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 0.0f, 0.0f },
			// EAST                                                      
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 0.0f, 1.0f },
			// NORTH                                                     
			{ 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 1.0f,    0.0f, 1.0f, 1.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 0.0f, 1.0f },
			// WEST                                                      
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 1.0f,    0.0f, 1.0f, 0.0f },
			{ 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f, 0.0f },
			// TOP                                                       
			{ 0.0f, 1.0f, 0.0f,    0.0f, 1.0f, 1.0f,    1.0f, 1.0f, 1.0f },
			{ 0.0f, 1.0f, 0.0f,    1.0f, 1.0f, 1.0f,    1.0f, 1.0f, 0.0f },
			// BOTTOM                                                    
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f },
			{ 1.0f, 0.0f, 1.0f,    0.0f, 0.0f, 0.0f,    1.0f, 0.0f, 0.0f }
		}
	};
	//meshCube.LoadFromObjectFile("VideoShip.obj");

	float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFov = 90.0f;
	float fAspectRatio = (float)device.height / (float)device.width;
	float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

	matProj = {
		fAspectRatio * fFovRad,0,0,0,
		0,fFovRad,0,0,
		0,0,fFar / (fFar - fNear),(-fFar * fNear) / (fFar - fNear),
		0,0,1,0
	};
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
	matRotZ = {
		cosf(fTheta),-sinf(fTheta),0,0,
		sinf(fTheta),cosf(fTheta),0,0,
		0,0,1,0,
		0,0,0,1
	};

	// Rotation X
	matRotX = {
		1,0,0,0,
		0,cosf(fTheta * 0.5f),-sinf(fTheta * 0.5f),0,
		0,sinf(fTheta * 0.5f),cosf(fTheta * 0.5f),0,
		0,0,0,1
	};

	// Store triagles for rastering later
	vector<triangle> vecTrianglesToRaster;

	// calc tri porjcet
	for (auto tri : meshCube.tris)
	{
		triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

		// Rotate in Z-Axis
		MultiplyMatrixVector(matRotZ, tri.p[0], triRotatedZ.p[0]);
		MultiplyMatrixVector(matRotZ, tri.p[1], triRotatedZ.p[1]);
		MultiplyMatrixVector(matRotZ, tri.p[2], triRotatedZ.p[2]);

		// Rotate in X-Axis
		MultiplyMatrixVector(matRotX, triRotatedZ.p[0], triRotatedZX.p[0]);
		MultiplyMatrixVector(matRotX, triRotatedZ.p[1], triRotatedZX.p[1]);
		MultiplyMatrixVector(matRotX, triRotatedZ.p[2], triRotatedZX.p[2]);

		// Offset into the screen
		float worldz = 7.0f;
		triTranslated = triRotatedZX;
		triTranslated.p[0].z = triRotatedZX.p[0].z + worldz;
		triTranslated.p[1].z = triRotatedZX.p[1].z + worldz;
		triTranslated.p[2].z = triRotatedZX.p[2].z + worldz;

		//printf("%f,%f,%f  %f,%f,%f\n", tri.p[0].z, tri.p[1].z, tri.p[2].z, triRotatedZ.p[0].z, triRotatedZ.p[1].z, triRotatedZ.p[2].z);

		//---------------------


		// Use Cross-Product to get surface normal
		vec3 normal, line1, line2;
		line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
		line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
		line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

		line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
		line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
		line2.z = triTranslated.p[2].z - triTranslated.p[0].z;
		// line1 cross line2
		normal.x = line1.y * line2.z - line1.z * line2.y;
		normal.y = line1.z * line2.x - line1.x * line2.z;
		normal.z = line1.x * line2.y - line1.y * line2.x;

		// It's normally normal to normalise the normal
		float l = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		normal.x /= l; normal.y /= l; normal.z /= l;
		// * unit vector dot is angle
		//if (normal.z < 0)
		if (normal.x * (triTranslated.p[0].x - vCamera.x) +
			normal.y * (triTranslated.p[0].y - vCamera.y) +
			normal.z * (triTranslated.p[0].z - vCamera.z) < 0.0f)
		{

			vec3 light_direction = { 0.0f, 0.0f, -1.0f };
			float l = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);
			light_direction.x /= l; light_direction.y /= l; light_direction.z /= l;
			// 光的向量與面向量夾角
			float dp = normal.x * light_direction.x + normal.y * light_direction.y + normal.z * light_direction.z;

			// Project triangles from 3D --> 2D
			MultiplyMatrixVector(matProj, triTranslated.p[0], triProjected.p[0]);
			MultiplyMatrixVector(matProj, triTranslated.p[1], triProjected.p[1]);
			MultiplyMatrixVector(matProj, triTranslated.p[2], triProjected.p[2]);

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
void MultiplyMatrixVector(mat4x4& m44, vec3& i, vec3& o)
{
	/*
	r\c
		[a b c d] |x|
		[e f g h] |y|
		[i j k l] |z|
		[m n o p] |w|
	m44[0][1]=>b
	m44[0][2]=>c
	*/
	o.x = i.x * m44.M[0][0] + i.y * m44.M[0][1] + i.z * m44.M[0][2] + m44.M[0][3];
	o.y = i.x * m44.M[1][0] + i.y * m44.M[1][1] + i.z * m44.M[1][2] + m44.M[1][3];
	o.z = i.x * m44.M[2][0] + i.y * m44.M[2][1] + i.z * m44.M[2][2] + m44.M[2][3];
	float w = i.x * m44.M[3][0] + i.y * m44.M[3][1] + i.z * m44.M[3][2] + m44.M[3][3];

	if (w != 0.0f)
	{
		o.x /= (float)w; o.y /= (float)w; o.z /= (float)w;
	}
}