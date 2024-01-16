#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "model.h"

Model::Model(const char* filename) : verts_(), faces_() {
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
            for (int i = 0; i < 3; i++) iss >> v.raw[i];
            
            verts_.push_back(v);
        }

        //获取纹理坐标
        else if (!line.compare(0, 2, "vt")) {
            iss >> trash;
            Vec2f vt;
            //由于vt后面有2个空格
            for (int i = 0; i < 2; i++) iss >>trash>> vt.raw[i];
           // std::cerr << "vt.x " << vt.x << std::endl;
            texverts_.push_back(vt);
        }

        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<float> tf;
            int itrash, idx;
            int tv;
            iss >> trash;
            while (iss >> idx >> trash >> tv >> trash >> itrash) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                tv--;
                //获取当前顶点的纹理位置
                tf.push_back(tv);
            }
            faces_.push_back(f);
            texfaces_.push_back(tf);
        }
    }
    std::cerr << "# v# " << verts_.size() << "# vt# " << texverts_.size() << "# f# " << faces_.size() << " tf# " << texfaces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::ntexverts()
{
    return (int)texverts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntexfaces()
{
    return (int)texfaces_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<float> Model::texface(int idx)
{
    return texfaces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::texvert(int i)
{
    return texverts_[i];
}
