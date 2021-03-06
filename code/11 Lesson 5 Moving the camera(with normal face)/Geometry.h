#pragma once
#include <strstream>
#include <math.h> 

typedef unsigned int UI32;

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


// https://stackoverflow.com/questions/42865608/no-operator-matches-these-operands-operand-types-are-const-vec2-float
struct _Vec2 {
	float x, y;

	_Vec2 operator=(const _Vec2& v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
		}
		return *this;
	}
	_Vec2 operator+(const _Vec2& v)const { return { x + v.x, y + v.y }; }
	_Vec2 operator-(const _Vec2& v)const { return { x - v.x, y - v.y }; }
	_Vec2 operator*(float f)const { return { x * f, y * f }; }
	float& operator[](const int i) { return i <= 0 ? x : y; }
};
struct _Vec3 {
	float x = 0, y = 0, z = 0, w = 1;
	_Vec3 operator=(const _Vec3& v) {
		if (this != &v) {
			x = v.x;
			y = v.y;
			z = v.z;
		}
		return *this;
	}
	_Vec3 operator^(const _Vec3& v) const { return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x }; }//cross
	_Vec3 operator+(const _Vec3& v)const { return { x + v.x, y + v.y, z + v.z }; }
	_Vec3 operator-(const _Vec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
	_Vec3 operator*(float f)const { return { x * f, y * f, z * f }; }
	float operator*(const _Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
	float norm() const { return std::sqrt(x * x + y * y + z * z); }// length
	_Vec3& normalize() { *this = (*this) * (1 / norm()); return *this; }
	float& operator[](const int i) { return i <= 0 ? x : (1 == i ? y : z); }
	_Vec3 tofloor() {
		return _Vec3{ floor(x) ,floor(y) ,floor(z) };
	}
};

typedef struct _Vec3 vec3;
typedef struct _Vec2 vec2;


typedef struct _Triangle {
	vec3 p[3];
	UI32 col;
} triangle;


vec3 Vector_Add(vec3& v1, vec3& v2);
vec3 Vector_Sub(vec3& v1, vec3& v2);
vec3 Vector_Mul(vec3& v1, float k);
vec3 Vector_Div(vec3& v1, float k);
float Vector_DotProduct(vec3& v1, vec3& v2);
float Vector_Length(vec3& v);
vec3 Vector_Normalise(vec3& v);
vec3 Vector_CrossProduct(vec3& v1, vec3& v2);
vec3 Vector_IntersectPlane(vec3& plane_p, vec3& plane_n, vec3& lineStart, vec3& lineEnd);
vec3 barycentric(vec3 a, vec3 b, vec3 c, vec3 p);

void MultiplyMatrixVector(mat4x4& m44, vec3& i, vec3& o);
vec3 Matrix_MultiplyVector(mat4x4& m44, vec3& i);
mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2);
mat4x4 Matrix_MakeIdentity();
mat4x4 Matrix_MakeRotationX(float fAngleRad);
mat4x4 Matrix_MakeRotationY(float fAngleRad);
mat4x4 Matrix_MakeRotationZ(float fAngleRad);
mat4x4 Matrix_MakeTranslation(float tx, float ty, float tz);
mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
mat4x4 Matrix_PointAt(vec3& pos, vec3& target, vec3& up);
mat4x4 Matrix_QuickInverse(mat4x4& m);

int Triangle_ClipAgainstPlane(vec3 plane_p, vec3 plane_n, triangle& ori_tri, triangle& ret_tri1, triangle& ret_tri2, bool debug);
void showMatrix(mat4x4 m);