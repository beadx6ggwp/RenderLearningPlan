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
			Vec3f v;
			//for (int i = 0; i < 3; i++) iss >> v[i];
			iss >> v.x >> v.y >> v.z;
			verts_.push_back(v);
		}
		else if (!line.compare(0, 3, "vn ")) {
			iss >> trash >> trash;
			Vec3f n;
			//for (int i = 0; i < 3; i++) iss >> n[i];
			iss >> n.x >> n.y >> n.z;
			norms_.push_back(n.normalize());
		}
		else if (!line.compare(0, 3, "vt ")) {
			iss >> trash >> trash;
			Vec2f uv;
			//for (int i = 0; i < 2; i++) iss >> uv[i];
			iss >> uv.x >> uv.y;
			uv_.push_back(uv);
		}
		else if (!line.compare(0, 2, "f ")) {
			vector<Vec3i> face;
			Vec3i tmp;
			int v, t, n;
			int cnt = 0;
			iss >> trash;

			//--------------
			// f 1 2 3, 先隨便寫個純頂點的輸入，之後記得整合
			int cCount = 0;
			for (int i = 0; i < line.length(); i++) {
				if (line[i] == '/')cCount++;
			}
			if (cCount == 0) {
				while (iss >> v ) {
					face.push_back(Vec3i(--v, -1, -1));
				}
				faces_.push_back(face);
				continue;
			}
			//------------
			
			// 連續頂點0 1 2，所以陣列中每三個index為一個面
			// facet_vrt_[i*3];
			while (iss >> v >> trash >> t >> trash >> n) {
				face.push_back(Vec3i(--v, --t, --n));
				cnt++;
			}
			faces_.push_back(face);
		}
	}
	in.close();
	std::cerr << "# v# " << nverts() << " f# " << nfaces() << " vt# " << uv_.size() << " vn# " << norms_.size() << std::endl;
	load_texture(filename, "_diffuse.tga", diffusemap_);
	load_texture(filename, "_nm.tga", normalmap_);
}

int Model::nverts() const {
	return verts_.size();
}

int Model::nfaces() const {
	return (int)faces_.size();
}

Vec3f Model::vert(int i) {
	return verts_[i];
}
Vec3f Model::vert(int iface, int nthvert) {
	return verts_[faces_[iface][nthvert][0]];
}

std::vector<int> Model::face(int idx) {
	vector<int> face;
	for (int i = 0; i < (int)faces_[idx].size(); i++)
		face.push_back(faces_[idx][i].x);//[0]
	return face;
}


Vec2f Model::uv(int iface, int nthvert) {
	// 12 lesson6 up, 原本是在取得uv時就直接取得img的位置，換成只先取得uv coord，在diffuse的時候才取得img pos
	//int idx = faces_[iface][nvert].y;//[1]
	//return Vec2f{ uv_[idx].x * diffusemap_.get_width(), uv_[idx].y * diffusemap_.get_height() };
	return uv_[faces_[iface][nthvert][1]];
}

Vec3f Model::normal(int iface, int nvert) {
	int idx = faces_[iface][nvert].z;//[2]
	return norms_[idx].normalize();
}

//Global coordinate/ Darboux frame(tangent space)
Vec3f Model::normal(Vec2f uvf) {
	Vec2i uv(uvf[0] * normalmap_.get_width(), uvf[1] * normalmap_.get_height());
	TGAColor c = normalmap_.get(uv[0], uv[1]);
	Vec3f res;
	// tcolor use bgra, so 2 - i, r=x g=y b=z
	// use global coordinate
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c[i] / 255.f * 2.f - 1.f; //rgb(0~255)重新映射回[-1,1]
	return res;// x,y,z = r,g,b
}

TGAColor Model::diffuse(Vec2f uvf) {
	//return diffusemap_.get(uv.x, uv.y);

	Vec2i uv(uvf[0] * diffusemap_.get_width(), uvf[1] * diffusemap_.get_height());
	return diffusemap_.get(uv[0], uv[1]);
}

float Model::specular(Vec2f uvf) {
	Vec2i uv(uvf[0] * specularmap_.get_width(), uvf[1] * specularmap_.get_height());
	return specularmap_.get(uv[0], uv[1])[0] / 1.f;
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