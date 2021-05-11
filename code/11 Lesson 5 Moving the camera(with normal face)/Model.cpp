#include "model.h"

#include <iostream>
#include <fstream>
#include <sstream>

Model::Model(const std::string filename) {
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (in.fail()) return;
	std::string line;
	while (!in.eof()) {
		std::getline(in, line);
		std::istringstream iss(line.c_str());
		char trash;
		if (!line.compare(0, 2, "v ")) {
			iss >> trash;
			vec3 v;
			//for (int i = 0; i < 3; i++) iss >> v[i];
			iss >> v.x >> v.y >> v.z;
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			vec3 n;
			//for (int i = 0; i < 3; i++) iss >> n[i];
			iss >> n.x >> n.y >> n.z;
			norms_.push_back(Vector_Normalise(n));
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			vec2 uv;
			//for (int i = 0; i < 2; i++) iss >> uv[i];
			iss >> uv.x >> uv.y;
			uv_.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {
			vector<faceData> f_arr;// test1
			int f, t, n;
			iss >> trash;
			int cnt = 0;
			// 連續頂點0 1 2，所以陣列中每三個index為一個面
			// facet_vrt_[i*3];
			while (iss >> f >> trash >> t >> trash >> n) {
				f_arr.push_back(faceData{ --f,--t,--n });
				cnt++;
			}
			faces_.push_back(f_arr);
		}
	}
	in.close();
	std::cerr << "# v# " << nverts() << " f# " << nfaces() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
	load_texture(filename, "_diffuse.tga", diffusemap_);
}

int Model::nverts() const {
	return verts_.size();
}

int Model::nfaces() const {
	return (int)faces_.size();
}

vec3 Model::vert(int i) {
	return verts_[i];
}

std::vector<int> Model::face(int idx) {
	vector<int> face;
	for (int i = 0; i < (int)faces_[idx].size(); i++) face.push_back(faces_[idx][i].v);
	return face;
}


vec2 Model::uv(int iface, int nvert) {
	int idx = faces_[iface][nvert].uv;
	return vec2{ uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height() };
}

vec3 Model::norm(int iface, int nvert) {
	int idx = faces_[iface][nvert].n;
	return norms_[idx].normalize();
}

TGAColor Model::diffuse(vec2 uv) {
	return diffusemap_.get(uv.x, uv.y);
}

void Model::load_texture(std::string filename, const char* suffix, TGAImage& img) {
	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos) {
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
		img.flip_vertically();
	}
}