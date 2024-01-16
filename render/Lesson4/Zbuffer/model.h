#pragma once
#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> texverts_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<float> > texfaces_;
public:
	Model(const char* filename);
	~Model();
	int nverts();
	int ntexverts();
	int nfaces();
	int ntexfaces();
	Vec3f vert(int i);
	Vec2f texvert(int i);
	std::vector<int> face(int idx);
	std::vector<float> texface(int idx);
};

#endif //__MODEL_H__