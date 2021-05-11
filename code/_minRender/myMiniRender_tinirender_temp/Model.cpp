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
			vector<int> f_arr;// test1
			int f, t, n;
			iss >> trash;
			int cnt = 0;
			// 連續頂點0 1 2，所以陣列中每三個index為一個面
			// facet_vrt_[i*3];
			while (iss >> f >> trash >> t >> trash >> n) {
				facet_vrt_.push_back(--f);
				f_arr.push_back(f);// test1
				facet_tex_.push_back(--t);
				facet_nrm_.push_back(--n);
				cnt++;
			}
			faces_.push_back(f_arr);// test1
			if (3 != cnt) {
				std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
				in.close();
				return;
			}
		}
	}
	in.close();
	std::cerr << "# v# " << nverts() << " f# " << nfaces() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
}

int Model::nverts() const {
	return verts_.size();
}

int Model::nfaces() const {
	return facet_vrt_.size() / 3;
}

vec3 Model::vert(int i) {
	return verts_[i];
}

std::vector<int> Model::face(int idx) {
	return faces_[idx];
}