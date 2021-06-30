#include <cmath>
#include <limits>
#include <cstdlib>

#include "my_gl.h"
#include "device.h"

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
	Projection[3][2] = coeff;
}

// target pos
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
	Vec3f z = (eye - center).normalize();
	Vec3f x = cross(up, z).normalize();
	Vec3f y = cross(z, x).normalize();
	ModelView = Matrix::identity();
	for (int i = 0; i < 3; i++) {
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];
		ModelView[i][3] = -center[i];
	}
	//ModelView[0][3] = center * x;
	//ModelView[1][3] = center * y;
	//ModelView[2][3] = center * z;
}

void triangle(Vec4f* pts, IShader& shader, Device &device) {
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