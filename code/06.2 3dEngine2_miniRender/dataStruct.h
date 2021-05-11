#pragma once

#include <fstream>
#include <strstream>
#include <vector>

typedef unsigned int u32;
typedef struct _Vec2 { float x, y; } vec2;
typedef struct _Vec3 { float x, y, z; } vec3;

typedef struct _Triangle { vec3 p[3]; } triangle;

typedef struct MATRIX4X4_TYP {
	union {
		float M[4][4];

		struct {
			float M00, M01, M02, M03;
			float M10, M11, M12, M13;
			float M20, M21, M22, M23;
			float M30, M31, M32, M33;
		};
	};
}mat4x4, * mat4x4_ptr;