#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
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
            //std::cerr << "v.y " << v[1] << std::endl;
            verts_.push_back(v);
        }

        //获取纹理坐标
        else if (!line.compare(0, 3, "vt ")) {
            iss >> trash >> trash;
            Vec2f vt;
            //由于vt后面有2个空格
            for (int i = 0; i < 2; i++) iss >> vt.raw[i];
            // std::cerr << "vt.x " << vt.x << std::endl;
            texverts_.push_back(vt);
        }

        //获取顶点法线坐标
        else if (!line.compare(0, 3, "vn ")) {
            iss >> trash >> trash;
            Vec3f vn;
            //由于vn后面有2个空格
            for (int i = 0; i < 3; i++) iss  >>  vn[i];
            //std::cerr << "vn.z " << vn.z << std::endl;
            texnormal_.push_back(vn);
        }

        else if (!line.compare(0, 2, "f ")) {
            std::vector<int> f;
            std::vector<int> tf;
            std::vector<int> tfn;
            int idx;
            int tv;
            int tn;
            iss >> trash;
            while (iss >> idx >> trash >> tv >> trash >> tn) {
                idx--; // in wavefront obj all indices start at 1, not zero
                f.push_back(idx);
                tv--;
                //获取当前顶点的纹理位置
                tf.push_back(tv);
                tn--;
                tfn.push_back(tn);
            }
            faces_.push_back(f);
            texfaces_.push_back(tf);
            texfacesnormal_.push_back(tfn);
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

int Model::ntexnomal()
{
    return (int)texnormal_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

int Model::ntexfaces()
{
    return (int)texfaces_.size();
}

int Model::ntexfacenormal()
{
    return (int)texfacesnormal_.size();
}

std::vector<int> Model::face(int idx) {
    return faces_[idx];
}

std::vector<int> Model::texface(int idx)
{
    return texfaces_[idx];
}

std::vector<int> Model::texfacenormal(int idx)
{
    return texfacesnormal_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec2f Model::texvert(int i)
{
    return texverts_[i];
}

Vec3f Model::texnormal(int i)
{
    return texnormal_[i];
}
