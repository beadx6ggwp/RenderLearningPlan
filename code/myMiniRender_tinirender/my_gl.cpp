#include <iostream>
#include <cmath>
#include <limits>
#include <cstdlib>

#include "my_gl.h"
#include "device.h"

// bit0:isdrawLine,bit1:isdrawTexture
int RENDER_MODE = 2;// 10
Matrix ModelView;
Matrix Viewport;
Matrix Projection;

IShader::~IShader() {}

void viewport(int x, int y, int w, int h) {
	Viewport = Matrix::identity();
	Viewport[0][3] = x + w / 2.f;
	Viewport[1][3] = y + h / 2.f;
	Viewport[2][3] = 255.f / 2.f;

	Viewport[0][0] = w / 2.f;
	Viewport[1][1] = -h / 2.f;
	Viewport[2][2] = 255.f / 2.f;
}

void projection(float coeff) {
	Projection = Matrix::identity();
	//Projection[3][2] = coeff;
	//------------------

	//float fFovDegrees = 90.0f;
	//float fAspectRatio = 800.0f / 800.0f;// h/w
	//float fNear = 0.1f;
	//float fFar = 1000.0f;

	//float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);

	//Projection.set_col(0, embed<4>(Vec3f(fAspectRatio * fFovRad, 0, 0), 0.0f));
	//Projection.set_col(1, embed<4>(Vec3f(0, fFovRad, 0), 0.0f));
	//Projection.set_col(2, embed<4>(Vec3f(0, 0, fFar / (fFar - fNear)), 1.0f));
	//Projection.set_col(3, embed<4>(Vec3f(0, 0, (-fFar * fNear) / (fFar - fNear)), 0.0f));
}

// target pos
void lookat(Vec3f pos, Vec3f target, Vec3f up) {
	Vec3f zaxis = (pos - target).normalize();
	Vec3f xaxis = cross(up, zaxis).normalize();
	Vec3f yaxis = cross(zaxis, xaxis).normalize();

	Matrix trans = Matrix::identity();
	trans.set_col(3, embed<4>(Vec3f(-pos.x, -pos.y, -pos.z)));

	Matrix rot = Matrix::identity();
	rot[0][0] = xaxis.x;
	rot[1][0] = xaxis.y;
	rot[2][0] = xaxis.z;

	rot[0][1] = yaxis.x;
	rot[1][1] = yaxis.y;
	rot[2][1] = yaxis.z;

	rot[0][2] = zaxis.x;
	rot[1][2] = zaxis.y;
	rot[2][2] = zaxis.z;

	ModelView = rot * trans;
}

void triangle(Vec4f* pts, IShader& shader, Device& device) {
	float t = device.zbuffer[0][0];

	// 如果物體很靠近視角卻沒有裁切，會使得w無限趨近於0，在後續的齊次座標轉平面座標(clip to view),pts[i][j] / pts[i][3]的時候
	// x/0.000000000001=接近無限大，變成bboxmax可能是無限大，導致一片三角形的迴圈根本渲染不完，導致卡死
	// 像是w=0.041473時, 齊次座標轉平面座標的bboxmax(3552.54 2620.88)=迴圈要跑接近9百萬次，這就是為甚麼會卡死了
	// 暫時使用，最爛的方法，只是防止w=0的除零問題，導致的無限走訪迴圈
	for (int i = 0; i < 3; i++) {
		float z = pts[i][3];
		if (abs(z) <= 0.1) {
			return;
		}
	}

	if (RENDER_MODE & 1) {
		device.drawTriangle(proj<3>(pts[0] / pts[0][3]),
							proj<3>(pts[1] / pts[1][3]),
							proj<3>(pts[2] / pts[2][3]),
							0x0);
	}
	if (!(RENDER_MODE & 2)) return;

	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}

	Vec2i P;
	UI32 color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f c = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			float z = pts[0][2] * c.x + pts[1][2] * c.y + pts[2][2] * c.z;
			float w = pts[0][3] * c.x + pts[1][3] * c.y + pts[2][3] * c.z;

			if (P.x >= device.width || P.y >= device.height || P.x < 0 || P.y < 0) continue;
			int frag_depth = std::max(0, std::min(255, int(z / w + .5)));
			float zbuffer = device.zbuffer[(int)P.y][(int)P.x];

			if (c.x < 0 || c.y < 0 || c.z<0 || zbuffer > frag_depth) continue;
			bool discard = shader.fragment(c, color);
			if (!discard) {
				device.zbuffer[(int)P.y][(int)P.x] = frag_depth;
				device.setPixel(P.x, P.y, color);
			}
		}
	}
}

void triangle2(Vec4f* pts, IShader& shader, Device& device) {
	float t = device.zbuffer[0][0];
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
			bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
		}
	}
	Vec2i P;
	UI32 color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
			Vec3f bc_screen = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
			Vec3f bc_clip = Vec3f(bc_screen.x / pts[0][3], bc_screen.y / pts[1][3], bc_screen.z / pts[2][3]);
			bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

			if (P.x >= device.width || P.y >= device.height || P.x < 0 || P.y < 0) continue;

			Vec3f temp(pts[0][2], pts[1][2], pts[2][2]);
			float  frag_depth = temp * bc_clip;
			float zbuffer = device.zbuffer[(int)P.y][(int)P.x];

			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z<0 || zbuffer > frag_depth) continue;
			bool discard = shader.fragment(bc_clip, color);
			if (!discard) {
				device.zbuffer[(int)P.y][(int)P.x] = frag_depth;
				device.setPixel(P.x, P.y, color);
			}
		}
	}
}