#pragma once

#include <vector>
#include <string>
#include "geometry.h"

using std::vector;
using std::string;

class Model
{
public:
	vector<vec3> verts_;     // array of vertices
	vector<vec2> uv_;        // array of tex coords
	vector<vec3> norms_;     // array of normal vectors
	vector<vector<int> > faces_;
	vector<int> facet_vrt_;
	vector<int> facet_tex_;  // indices in the above arrays per triangle
	vector<int> facet_nrm_;

	Model(const string filename);
	int nverts() const;
	int nfaces() const;
	vec3 vert(int i);
	vector<int> face(int idx);
};

