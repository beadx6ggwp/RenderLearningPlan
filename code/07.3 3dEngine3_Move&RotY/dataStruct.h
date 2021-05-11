#pragma once

#include <fstream>
#include <strstream>
#include <vector>

typedef unsigned int u32;
typedef struct _Vec2 { float x, y; } vec2;
typedef struct _Vec3 { float x = 0, y = 0, z = 0, w = 1; } vec3;

typedef struct _Triangle {
	vec3 p[3];
	u32 col;
} triangle;

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


vec3 Vector_Add(vec3& v1, vec3& v2)
{
	return { v1.x + v2.x, v1.y + v2.y, v1.z + v2.z };
}

vec3 Vector_Sub(vec3& v1, vec3& v2)
{
	return { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
}

vec3 Vector_Mul(vec3& v1, float k)
{
	return { v1.x * k, v1.y * k, v1.z * k };
}

vec3 Vector_Div(vec3& v1, float k)
{
	return { v1.x / k, v1.y / k, v1.z / k };
}

float Vector_DotProduct(vec3& v1, vec3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vector_Length(vec3& v)
{
	return sqrtf(Vector_DotProduct(v, v));
}

vec3 Vector_Normalise(vec3& v)
{
	float l = Vector_Length(v);
	return { v.x / l, v.y / l, v.z / l };
}

vec3 Vector_CrossProduct(vec3& v1, vec3& v2)
{
	vec3 v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
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

vec3 Matrix_MultiplyVector(mat4x4& m44, vec3& i) {
	vec3 v;
	v.x = i.x * m44.M[0][0] + i.y * m44.M[0][1] + i.z * m44.M[0][2] + m44.M[0][3];
	v.y = i.x * m44.M[1][0] + i.y * m44.M[1][1] + i.z * m44.M[1][2] + m44.M[1][3];
	v.z = i.x * m44.M[2][0] + i.y * m44.M[2][1] + i.z * m44.M[2][2] + m44.M[2][3];
	v.w = i.x * m44.M[3][0] + i.y * m44.M[3][1] + i.z * m44.M[3][2] + m44.M[3][3];
	return v;
}

mat4x4 Matrix_MakeIdentity()
{
	mat4x4 matrix = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	return matrix;
}


mat4x4 Matrix_MakeRotationX(float fAngleRad)
{
	mat4x4 matrix = {
		1,0,0,0,
		0,cosf(fAngleRad),-sinf(fAngleRad),0,
		0,sinf(fAngleRad),cosf(fAngleRad),0,
		0,0,0,1
	};
	return matrix;
}
mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
	mat4x4 matrix = {
		cosf(fAngleRad),0,sinf(fAngleRad),0,
		0,1,0,0,
		-sinf(fAngleRad),0,cosf(fAngleRad),0,
		0,0,0,1
	};
	return matrix;
}
mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
	mat4x4 matrix = {
		cosf(fAngleRad),-sinf(fAngleRad),0,0,
		sinf(fAngleRad),cosf(fAngleRad),0,0,
		0,0,1,0,
		0,0,0,1
	};
	return matrix;
}

mat4x4 Matrix_MakeTranslation(float tx, float ty, float tz)
{
	mat4x4 matrix = {
		1,0,0,tx,
		0,1,0,ty,
		0,0,1,tz,
		0,0,0,1
	};
	return matrix;
}

// 視角、長寬比、近截面、遠截面
mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar)
{
	float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
	mat4x4 matrix = {
		fAspectRatio * fFovRad,0,0,0,
		0,fFovRad,0,0,
		0,0,fFar / (fFar - fNear),(-fFar * fNear) / (fFar - fNear),
		0,0,1,0
	};
	return matrix;
}

mat4x4 Matrix_PointAt(vec3& pos, vec3& target, vec3& up)
{
	// Calculate new forward direction
	vec3 newForward = Vector_Sub(target, pos);
	newForward = Vector_Normalise(newForward);

	// Calculate new Up direction
	vec3 a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
	vec3 newUp = Vector_Sub(up, a);
	newUp = Vector_Normalise(newUp);

	// New Right direction is easy, its just cross product
	vec3 newRight = Vector_CrossProduct(newUp, newForward);

	// Construct Dimensioning and Translation Matrix	
	mat4x4 matrix = {
		newRight.x,newUp.x,newForward.x,pos.x,
		newRight.y,newUp.y,newForward.y,pos.y,
		newRight.z,newUp.z,newForward.z,pos.z,
		0,0,0,1
	};
	//matrix.m[0][0] = newRight.x;	matrix.m[0][1] = newRight.y;	matrix.m[0][2] = newRight.z;	matrix.m[0][3] = 0.0f;
	//matrix.m[1][0] = newUp.x;		matrix.m[1][1] = newUp.y;		matrix.m[1][2] = newUp.z;		matrix.m[1][3] = 0.0f;
	//matrix.m[2][0] = newForward.x;	matrix.m[2][1] = newForward.y;	matrix.m[2][2] = newForward.z;	matrix.m[2][3] = 0.0f;
	//matrix.m[3][0] = pos.x;			matrix.m[3][1] = pos.y;     matrix.m[3][2] = pos.z;			matrix.m[3][3] = 1.0f;
	return matrix;

}

mat4x4 Matrix_QuickInverse(mat4x4& m) // Only for Rotation/Translation Matrices
{
	mat4x4 matrix = {
		m.M[0][0],	m.M[1][0],	m.M[2][0],	0,
		m.M[0][1],	m.M[1][1],	m.M[2][1],	0,
		m.M[0][2],	m.M[1][2],	m.M[2][2],	0,
		0,			0,			0,			1
	};
	matrix.M[0][3] = -(matrix.M[0][0] * m.M[0][3] + matrix.M[0][1] * m.M[1][3] + matrix.M[0][2] * m.M[2][3]);
	matrix.M[1][3] = -(matrix.M[1][0] * m.M[0][3] + matrix.M[1][1] * m.M[1][3] + matrix.M[1][2] * m.M[2][3]);
	matrix.M[2][3] = -(matrix.M[2][0] * m.M[0][3] + matrix.M[2][1] * m.M[1][3] + matrix.M[2][2] * m.M[2][3]);
	//matrix.M[0][0] = m.M[0][0]; matrix.M[0][1] = m.M[1][0]; matrix.M[0][2] = m.M[2][0]; matrix.M[0][3] = 0.0f;
	//matrix.M[1][0] = m.M[0][1]; matrix.M[1][1] = m.M[1][1]; matrix.M[1][2] = m.M[2][1]; matrix.M[1][3] = 0.0f;
	//matrix.M[2][0] = m.M[0][2]; matrix.M[2][1] = m.M[1][2]; matrix.M[2][2] = m.M[2][2]; matrix.M[2][3] = 0.0f;
	//matrix.M[3][0] = -(m.M[3][0] * matrix.M[0][0] + m.M[3][1] * matrix.M[1][0] + m.M[3][2] * matrix.M[2][0]);
	//matrix.M[3][1] = -(m.M[3][0] * matrix.M[0][1] + m.M[3][1] * matrix.M[1][1] + m.M[3][2] * matrix.M[2][1]);
	//matrix.M[3][2] = -(m.M[3][0] * matrix.M[0][2] + m.M[3][1] * matrix.M[1][2] + m.M[3][2] * matrix.M[2][2]);
	//matrix.M[3][3] = 1.0f;
	return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2)
{
	/*mat4x4 m1 = {
		1,2,3,4,
		5,6,7,8,
		9,10,11,12,
		13,14,15,16
	};
	mat4x4 m2 = {
		1,1,1,1,
		2,2,2,2,
		3,3,3,3,
		4,4,4,4
	};
	mat4x4 m3 = Matrix_MultiplyMatrix(m1, m2);*/
	mat4x4 matrix;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			matrix.M[i][j] =
			m1.M[i][0] * m2.M[0][j] +
			m1.M[i][1] * m2.M[1][j] +
			m1.M[i][2] * m2.M[2][j] +
			m1.M[i][3] * m2.M[3][j];
	return matrix;
}

void showMatrix(mat4x4 m) {
	printf("------\n");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			printf("%.3f\t", m.M[i][j]);
		}
		printf("\n");
	}
	printf("------\n");
}