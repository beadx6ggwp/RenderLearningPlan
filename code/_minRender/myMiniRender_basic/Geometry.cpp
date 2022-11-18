#include "Geometry.h"
#include <stdio.h>

template <> template <> vec<3, int>  ::vec(const vec<3, float>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> vec<3, float>::vec(const vec<3, int>& v) : x(v.x), y(v.y), z(v.z) {}
template <> template <> vec<2, int>  ::vec(const vec<2, float>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> vec<2, float>::vec(const vec<2, int>& v) : x(v.x), y(v.y) {}

////////////////////////////////////////////////

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
	Vec3f s[2];
	for (int i = 2; i--; ) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}
	Vec3f u = cross(s[0], s[1]);
	// 因為cross出來的結果應該要是(x,y,z) = u,v,1，但也可能不是1，所以需要(x/z, y/z/, z/z)，怕會有除0問題加z為0本身就是異常，所以做個判斷
	if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}



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

////////////////////////////////////////////////

void MultiplyMatrixVector(mat4x4& m44, Vec3f& i, Vec3f& o)
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

Vec3f Matrix_MultiplyVector(mat4x4& m44, Vec3f& i) {
	Vec3f v;
	v.x = i.x * m44.M[0][0] + i.y * m44.M[0][1] + i.z * m44.M[0][2] + m44.M[0][3];
	v.y = i.x * m44.M[1][0] + i.y * m44.M[1][1] + i.z * m44.M[1][2] + m44.M[1][3];
	v.z = i.x * m44.M[2][0] + i.y * m44.M[2][1] + i.z * m44.M[2][2] + m44.M[2][3];
	//v.w = i.x * m44.M[3][0] + i.y * m44.M[3][1] + i.z * m44.M[3][2] + m44.M[3][3];
	return v;
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
////////////////////////////////////////

//////////////////////////////////////


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