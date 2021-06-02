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
	vector<vector<faceData> > faces_;
	TGAImage diffusemap_;

	Model(const string filename);
	int nverts() const;
	int nfaces() const;
	Vec3f vert(int i);
	vector<int> face(int idx);
	Vec2f uv(int iface, int nvert);
	Vec3f norm(int iface, int nvert);
	TGAColor diffuse(Vec2f uv);
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
};

