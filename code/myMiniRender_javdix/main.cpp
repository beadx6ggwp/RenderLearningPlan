#include "device.h"
#include <chrono> // timer
#include <list>
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
				int count = 0;
				for (int t = 0; line[t] != '\0'; t++) {
					if (line[t] == 32)count++;
				}
				//tri mesh
				if (count == 3) {
					int f[3];
					s >> junk >> f[0] >> f[1] >> f[2];
					int f0 = f[0] - 1;
					int f1 = f[1] - 1;
					int f2 = f[2] - 1;
					tris.push_back({ verts[f0], verts[f1], verts[f2] });
				}
				if (count == 4) {
					int f[4];
					s >> junk >> f[0] >> f[1] >> f[2] >> f[3];
					int f0 = f[0] - 1;
					int f1 = f[1] - 1;
					int f2 = f[2] - 1;
					int f3 = f[3] - 1;
					tris.push_back({ verts[f0], verts[f1], verts[f2] });
					tris.push_back({ verts[f0], verts[f2], verts[f3] });
				}
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

int isKeydown[512] = { 0 };
int showMode = 0, showClipping = 0;

vec3 vCamera;
vec3 vLookDir = { 0,0,1 };
float fYaw;

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
	device.background = 0xAAAAAA;

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
	meshCube = {
		{
			// SOUTH
			{ 0.0f, 0.0f, -1.0f, 1.0f,    -2.0f, 0.0f, 1.0f, 1.0f,    4.0f, 0.0f, 1.0f, 1.0f, }
		}
	};
	//meshCube = {
	//	{
	//		// SOUTH
	//		{ 0.0f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f, },
	//		{ 0.0f, 0.0f, 0.0f, 1.0f,    1.0f, 1.0f, 0.0f, 1.0f,    1.0f, 0.0f, 0.0f, 1.0f, },
	//	}
	//};
	//meshCube.LoadFromObjectFile("../_objfile/VideoShip.obj");
	//meshCube.LoadFromObjectFile("../_objfile/teapot.obj");
	//meshCube.LoadFromObjectFile("../_objfile/axis.obj");
	//meshCube.LoadFromObjectFile("../_objfile/house2.obj");
	//meshCube.LoadFromObjectFile("../_objfile/mountains.obj");

	// create view project
	matProj = Matrix_MakeProjection(90.0f, (float)device.height / (float)device.width, 0.1f, 1000.0f);
}

void gameMain() {
	fpsCounting();
	swprintf(strBuffer, 100, 
			 L"Win32 Render %dx%d, FPS:%4d, dt:%2dms showMode:%d clipping:%d", 
			 device.width, device.height, fps, dt,showMode,showClipping);
	SetWindowText(screen_handle, strBuffer);
	update();
	render();
}

void update() {
	if (screen_keys['L']) {
		if (!isKeydown['L']) {
			showMode = (showMode + 1) % 3;
			isKeydown['L'] = 1;
		}
	}
	else {
		isKeydown['L'] = 0;
	}
	if (screen_keys['O']) {
		if (!isKeydown['O']) {
			showClipping = !showClipping;
			isKeydown['O'] = 1;
		}
	}
	else {
		isKeydown['O'] = 0;
	}

	if (screen_keys['R']) {
		vCamera = { 0,0,0 };
		fYaw = 0;
	}
	if (screen_keys[VK_UP]) {
		vCamera.y += 10.0f * dt / 1000;
	}
	if (screen_keys[VK_DOWN]) {
		vCamera.y -= 10.0f * dt / 1000;
	}
	if (screen_keys[VK_RIGHT]) {
		//vCamera.x += 10.0f * dt / 1000;
		// get vLookDir right normal vec
		vec3 vRight = { vLookDir.z,0,-vLookDir.x };
		vec3 newMove = Vector_Mul(vRight, 10.0 * dt / 1000);
		vCamera = Vector_Add(vCamera, newMove);

	}
	if (screen_keys[VK_LEFT]) {
		//vCamera.x -= 10.0f * dt / 1000;
		// get vLookDir left normal vec
		vec3 vLeft = { -vLookDir.z,0,vLookDir.x };
		vec3 newMove = Vector_Mul(vLeft, 10.0 * dt / 1000);
		vCamera = Vector_Add(vCamera, newMove);
	}

	vec3 vForward = Vector_Mul(vLookDir, 10.0 * dt / 1000);

	if (screen_keys['W']) {
		vCamera = Vector_Add(vCamera, vForward);
	}
	if (screen_keys['S']) {
		vCamera = Vector_Sub(vCamera, vForward);
	}
	if (screen_keys['A']) {
		fYaw -= 1.0f * dt / 1000;
	}
	if (screen_keys['D']) {
		fYaw += 1.0f * dt / 1000;
	}
	printf("Camera: %.3f, %.3f, %.3f  RotYa:%.3f\n", vCamera.x, vCamera.y, vCamera.z, fYaw * 180.0f / 3.14f);
}
void render() {
	device_clear(&device, 0);

	// -----------
	//fillTriangle(&device, v1, v2, v3, 0xFF7700);
	//simpleMsaaTriangle(&device, v1, v2, v3, 0xFF7700);
	//drawTriangle(&device, v1, v2, v3, 0x000000);

	mat4x4 matRotZ, matRotX, matScale;
	//fTheta += 0.7f * dt / 1000;

	// Rotation Z
	matRotZ = Matrix_MakeRotationZ(fTheta * 1.0f);

	// Rotation X
	matRotX = Matrix_MakeRotationX(fTheta * 0.5f);

	mat4x4 matTrans;
	matTrans = Matrix_MakeTranslation(0.0, 0.0f, 15.0f);

	// from modle to world pos
	// 繞軸旋轉順序 y>x>z or z>y>x
	// here is z>y>x
	mat4x4 matWorld;
	matWorld = Matrix_MakeIdentity(); //init
	matWorld = Matrix_MultiplyMatrix(matRotZ, matWorld); // Transform by rotation
	matWorld = Matrix_MultiplyMatrix(matRotX, matWorld); // Transform by rotation
	matWorld = Matrix_MultiplyMatrix(matTrans, matWorld); // Transform by translation;
	// 先旋轉再平移，由pos開始往外乘，跟巢狀func一樣:T*(Rx*(Rz* pos)))，(AB)C=A(BC)

	vec3 vUp = { 0,1,0 };
	//vec3 vTarget = Vector_Add(vCamera, vLookDir);// 放一個目標在攝影機前方
	vec3 vTarget = { 0,0,1 };// 基準位置
	mat4x4 matCameraRotY = Matrix_MakeRotationY(fYaw);
	vLookDir = Matrix_MultiplyVector(matCameraRotY, vTarget);
	vTarget = Vector_Add(vCamera, vLookDir);

	mat4x4 matCamera = Matrix_PointAt(vCamera, vTarget, vUp);
	mat4x4 matView = Matrix_QuickInverse(matCamera);



	// Store triagles for rastering later
	vector<triangle> vecTrianglesToRaster;

	// calc tri porjcet
	for (auto tri : meshCube.tris)
	{
		triangle triProjected, triTransformed, triViewed;

		triTransformed.p[0] = Matrix_MultiplyVector(matWorld, tri.p[0]);
		triTransformed.p[1] = Matrix_MultiplyVector(matWorld, tri.p[1]);
		triTransformed.p[2] = Matrix_MultiplyVector(matWorld, tri.p[2]);

		//---------------------


		// Use Cross-Product to get tri surface normal
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

			vec3 light_direction = { 1.0f, 1.0f, -1.0f };
			light_direction = Vector_Normalise(light_direction);
			// 光的向量與面向量夾角
			float dp = max(0.1f, Vector_DotProduct(light_direction, normal));
			// calc light
			float cc = dp * 255;
			u32 c1 = ((u32)cc) << 16;
			//c1 = 0xff0000;
			triTransformed.col = c1;

			// world to view
			triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
			triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
			triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
			triViewed.col = triTransformed.col;

			// Clip Viewed Triangle against near plane, this could form two additional
			int nClippedTriangles = 0;
			triangle clipped[2];
			nClippedTriangles = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 2.1f }, { 0.0f, 0.0f, 1.0f }, triViewed, clipped[0], clipped[1], showClipping);

			for (int n = 0; n < nClippedTriangles; n++)
			{

				// Project triangles from 3D --> 2D
				triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
				triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
				triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
				triProjected.col = clipped[n].col;
				// Scale into view, we moved the normalising into cartesian space
				// out of the matrix.vector function from the previous videos, so
				// do this manually
				triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);


				// 從正規化空間 Scale into view
				// 先將Y軸反轉，因為電腦繪圖時Y軸是反的
				triProjected.p[0].x *= 1.0f;
				triProjected.p[1].x *= 1.0f;
				triProjected.p[2].x *= 1.0f;
				triProjected.p[0].y *= -1.0f;
				triProjected.p[1].y *= -1.0f;
				triProjected.p[2].y *= -1.0f;

				vec3 vOffsetView = { 1,1,0 };
				triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
				triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
				triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

				triProjected.p[0].x *= 0.5f * (float)device.width;
				triProjected.p[0].y *= 0.5f * (float)device.height;
				triProjected.p[1].x *= 0.5f * (float)device.width;
				triProjected.p[1].y *= 0.5f * (float)device.height;
				triProjected.p[2].x *= 0.5f * (float)device.width;
				triProjected.p[2].y *= 0.5f * (float)device.height;



				// Store triangle for sorting
				vecTrianglesToRaster.push_back(triProjected);
			}
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
	for (auto& triToRaster : vecTrianglesToRaster)
	{
		// Clip triangles against all four screen edges, this could yield
		// a bunch of triangles, so create a queue that we traverse to 
		//  ensure we only test new triangles generated against planes
		triangle clipped[2];
		list<triangle> listTriangles;

		// Add initial triangle
		listTriangles.push_back(triToRaster);
		int nNewTriangles = 1;

		// 螢幕上下左右四個面
		for (int p = 0; p < 4; p++)
		{
			int nTrisToAdd = 0;
			while (nNewTriangles > 0)
			{
				// Take triangle from front of queue
				triangle test = listTriangles.front();
				listTriangles.pop_front();
				nNewTriangles--;

				// Clip it against a plane. We only need to test each 
				// subsequent plane, against subsequent new triangles
				// as all triangles after a plane clip are guaranteed
				// to lie on the inside of the plane. I like how this
				// comment is almost completely and utterly justified
				switch (p)
				{
					case 0:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, test, clipped[0], clipped[1], showClipping); break;
					case 1:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, (float)device.height - 1, 0.0f }, { 0.0f, -1.0f, 0.0f }, test, clipped[0], clipped[1], showClipping); break;
					case 2:	nTrisToAdd = Triangle_ClipAgainstPlane({ 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], showClipping); break;
					case 3:	nTrisToAdd = Triangle_ClipAgainstPlane({ (float)device.width - 1, 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, test, clipped[0], clipped[1], showClipping); break;
				}
				// Clipping may yield a variable number of triangles, so
				// add these new ones to the back of the queue for subsequent
				// clipping against next planes
				for (int w = 0; w < nTrisToAdd; w++)
					listTriangles.push_back(clipped[w]);
			}
			nNewTriangles = listTriangles.size();
		}


		// Draw the transformed, viewed, clipped, projected, sorted, clipped triangles
		for (auto& tri : listTriangles)
		{
			// Rasterize triangle
			switch (showMode)
			{
				case 0:
					fillTriangle2(&device, tri.p[0], tri.p[1], tri.p[2], tri.col);
					break;
				case 1:
					//drawTriangle(&device, tri.p[0], tri.p[1], tri.p[2], 0);
					//drawLine(&device, tri.p[0].x, tri.p[0].y, tri.p[1].x, tri.p[1].y, 0xff0000);
					//drawLine(&device, tri.p[1].x, tri.p[1].y, tri.p[2].x, tri.p[2].y, 0x00ff00);
					//drawLine(&device, tri.p[2].x, tri.p[2].y, tri.p[0].x, tri.p[0].y, 0x0000ff);
					drawDebugTriangleLine(&device, tri.p[0], tri.p[1], tri.p[2]);
					break;
				case 2: {
					fillTriangle2(&device, tri.p[0], tri.p[1], tri.p[2], tri.col);
					drawTriangle(&device, tri.p[0], tri.p[1], tri.p[2], 0);
					break;
				}
			}
		}
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