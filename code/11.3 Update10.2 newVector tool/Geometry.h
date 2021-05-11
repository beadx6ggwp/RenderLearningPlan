#pragma once
#include <strstream>
#include <cassert>
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




template<size_t DimCols, size_t DimRows, typename T> class mat;

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i = DIM; i--; data_[i] = T()); }
    T& operator[](const size_t i) { assert(i < DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i < DIM); return data_[i]; }
private:
    T data_[DIM];
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<2, T> {
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec<2, T>(const vec<2, U>& v);
    T& operator[](const size_t i) { assert(i < 2); return i <= 0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i < 2); return i <= 0 ? x : y; }

    T x, y;
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<3, T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template <class U> vec<3, T>(const vec<3, U>& v);
    T& operator[](const size_t i) { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    const T& operator[](const size_t i) const { assert(i < 3); return i <= 0 ? x : (1 == i ? y : z); }
    float norm() { return std::sqrt(x * x + y * y + z * z); }
    vec<3, T>& normalize(T l = 1) { *this = (*this) * (l / norm()); return *this; }

    T x, y, z;
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM, typename T> T operator*(const vec<DIM, T>& lhs, const vec<DIM, T>& rhs) {
    T ret = T();
    for (size_t i = DIM; i--; ret += lhs[i] * rhs[i]);
    return ret;
}


template<size_t DIM, typename T>vec<DIM, T> operator+(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = DIM; i--; lhs[i] += rhs[i]);
    return lhs;
}

template<size_t DIM, typename T>vec<DIM, T> operator-(vec<DIM, T> lhs, const vec<DIM, T>& rhs) {
    for (size_t i = DIM; i--; lhs[i] -= rhs[i]);
    return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator*(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = DIM; i--; lhs[i] *= rhs);
    return lhs;
}

template<size_t DIM, typename T, typename U> vec<DIM, T> operator/(vec<DIM, T> lhs, const U& rhs) {
    for (size_t i = DIM; i--; lhs[i] /= rhs);
    return lhs;
}

template<size_t LEN, size_t DIM, typename T> vec<LEN, T> embed(const vec<DIM, T>& v, T fill = 1) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = (i < DIM ? v[i] : fill));
    return ret;
}

template<size_t LEN, size_t DIM, typename T> vec<LEN, T> proj(const vec<DIM, T>& v) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = v[i]);
    return ret;
}

template <typename T> vec<3, T> cross(vec<3, T> v1, vec<3, T> v2) {
    return vec<3, T>(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v) {
    for (unsigned int i = 0; i < DIM; i++) {
        out << v[i] << " ";
    }
    return out;
}

typedef vec<2, float> Vec2f;
typedef vec<2, int>   Vec2i;
typedef vec<3, float> Vec3f;
typedef vec<3, int>   Vec3i;
typedef vec<4, float> Vec4f;


typedef struct _Triangle {
	Vec3f p[3];
	UI32 col;
} triangle;

Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P);


void MultiplyMatrixVector(mat4x4& m44, Vec3f& i, Vec3f& o);
Vec3f Matrix_MultiplyVector(mat4x4& m44, Vec3f& i);
mat4x4 Matrix_MultiplyMatrix(mat4x4& m1, mat4x4& m2);
mat4x4 Matrix_MakeIdentity();
mat4x4 Matrix_MakeRotationX(float fAngleRad);
mat4x4 Matrix_MakeRotationY(float fAngleRad);
mat4x4 Matrix_MakeRotationZ(float fAngleRad);
mat4x4 Matrix_MakeTranslation(float tx, float ty, float tz);
mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
mat4x4 Matrix_PointAt(Vec3f& pos, Vec3f& target, Vec3f& up);
mat4x4 Matrix_QuickInverse(mat4x4& m);

int Triangle_ClipAgainstPlane(Vec3f plane_p, Vec3f plane_n, triangle& ori_tri, triangle& ret_tri1, triangle& ret_tri2, bool debug);
void showMatrix(mat4x4 m);