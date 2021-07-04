#pragma once

#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

using std::vector;
using std::string;

typedef struct { int v, uv, n; }faceData;

class Model
{
public:
	vector<Vec3f> verts_;     // array of vertices
	vector<Vec2f> uv_;        // array of tex coords
	vector<Vec3f> norms_;     // array of normal vectors
	vector<std::vector<Vec3i> > faces_; // Vec3i= vertex/uv/normal

	TGAImage diffusemap_;
	TGAImage normalmap_;
	TGAImage specularmap_;

	Model(const string filename);
	int nverts() const;
	int nfaces() const;
	Vec3f vert(int i);
	Vec3f vert(int iface, int nthvert);
	vector<int> face(int idx);

	Vec2f uv(int iface, int nvert);
	Vec3f normal(int iface, int nthvert);
	Vec3f normal(Vec2f uvf);
    float specular(Vec2f uvf);

	TGAColor diffuse(Vec2f uvf);
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
};

