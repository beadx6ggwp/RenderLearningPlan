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

//(平面P, 朝向的法向量N, 待測線段起點, 待測線段終點) return intersect Point
vec3 Vector_IntersectPlane(vec3& plane_p, vec3& plane_n, vec3& lineStart, vec3& lineEnd)
{
	plane_n = Vector_Normalise(plane_n);
	float plane_d = -Vector_DotProduct(plane_n, plane_p);
	float ad = Vector_DotProduct(lineStart, plane_n);
	float bd = Vector_DotProduct(lineEnd, plane_n);
	float t = (-plane_d - ad) / (bd - ad);
	vec3 lineStartToEnd = Vector_Sub(lineEnd, lineStart);
	vec3 lineToIntersect = Vector_Mul(lineStartToEnd, t);
	return Vector_Add(lineStart, lineToIntersect);
}

int Triangle_ClipAgainstPlane(vec3 plane_p, vec3 plane_n, triangle& ori_tri, triangle& ret_tri1, triangle& ret_tri2)
{
	// Make sure plane normal is indeed normal
	plane_n = Vector_Normalise(plane_n);

	// Return signed shortest distance from point to plane, plane normal must be normalised
	auto dist = [&](vec3& p)
	{
		vec3 n = Vector_Normalise(p);
		return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
	};

	// Create two temporary storage arrays to classify points either side of plane
	// If distance sign is positive, point lies on "inside" of plane
	vec3* inside_points[3];  int nInsidePointCount = 0;
	vec3* outside_points[3]; int nOutsidePointCount = 0;

	// Get signed distance of each point in triangle to plane
	float d0 = dist(ori_tri.p[0]);
	float d1 = dist(ori_tri.p[1]);
	float d2 = dist(ori_tri.p[2]);

	if (d0 >= 0) { inside_points[nInsidePointCount++] = &ori_tri.p[0]; }
	else { outside_points[nOutsidePointCount++] = &ori_tri.p[0]; }
	if (d1 >= 0) { inside_points[nInsidePointCount++] = &ori_tri.p[1]; }
	else { outside_points[nOutsidePointCount++] = &ori_tri.p[1]; }
	if (d2 >= 0) { inside_points[nInsidePointCount++] = &ori_tri.p[2]; }
	else { outside_points[nOutsidePointCount++] = &ori_tri.p[2]; }

	// Now classify triangle points, and break the input triangle into 
	// smaller output triangles if required. There are four possible
	// outcomes...

	if (nInsidePointCount == 0)
	{
		// All points lie on the outside of plane, so clip whole triangle
		// It ceases to exist

		return 0; // No returned triangles are valid
	}

	if (nInsidePointCount == 3)
	{
		// All points lie on the inside of plane, so do nothing
		// and allow the triangle to simply pass through
		ret_tri1 = ori_tri;

		return 1; // Just the one returned original triangle is valid
	}

	if (nInsidePointCount == 1 && nOutsidePointCount == 2)
	{
		// Triangle should be clipped. As two points lie outside
		// the plane, the triangle simply becomes a smaller triangle

		// Copy appearance info to new triangle
		ret_tri1.col = ori_tri.col;
		// debug
		ret_tri1.col = 0x0000ff;

		// The inside point is valid, so keep that...
		ret_tri1.p[0] = *inside_points[0];

		// but the two new points are at the locations where the 
		// original sides of the triangle (lines) intersect with the plane
		ret_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		ret_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1]);

		return 1; // Return the newly formed single triangle
	}

	if (nInsidePointCount == 2 && nOutsidePointCount == 1)
	{
		// Triangle should be clipped. As two points lie inside the plane,
		// the clipped triangle becomes a "quad". Fortunately, we can
		// represent a quad with two new triangles

		// Copy appearance info to new triangles
		ret_tri1.col = ori_tri.col;

		ret_tri2.col = ori_tri.col;


		// debug
		ret_tri1.col = 0x00ff00;
		ret_tri2.col = 0xff0000;


		vec3 v1 = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0]);
		vec3 v2 = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0]);
		// The first triangle consists of the two inside points and a new
		// point determined by the location where one side of the triangle
		// intersects with the plane
		ret_tri1.p[0] = *inside_points[0];
		ret_tri1.p[1] = *inside_points[1];
		ret_tri1.p[2] = v1;

		// The second triangle is composed of one of he inside points, a
		// new point determined by the intersection of the other side of the 
		// triangle and the plane, and the newly created point above
		ret_tri2.p[0] = *inside_points[1];
		ret_tri2.p[1] = v2;
		ret_tri2.p[2] = v1;

		// change v1 v2 保持以順時針建立三角形
		//ret_tri2.p[0] = *inside_points[1];
		//ret_tri2.p[1] = v1;
		//ret_tri2.p[2] = v2;

		return 2; // Return two newly formed triangles which form a quad
	}
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