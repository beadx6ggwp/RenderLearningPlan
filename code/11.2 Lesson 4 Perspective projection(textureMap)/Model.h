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
	vector<vec3> verts_;     // array of vertices
	vector<vec2> uv_;        // array of tex coords
	vector<vec3> norms_;     // array of normal vectors
	vector<vector<faceData> > faces_;
	TGAImage diffusemap_;

	Model(const string filename);
	int nverts() const;
	int nfaces() const;
	vec3 vert(int i);
	vector<int> face(int idx);
	vec2 uv(int iface, int nvert);
	vec3 norm(int iface, int nvert);
	TGAColor diffuse(vec2 uv);
	void load_texture(std::string filename, const char* suffix, TGAImage& img);
};

