#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include "geometry.h"

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec2f> texverts_;
	std::vector<Vec3f> texnormal_;
	std::vector<std::vector<int> > faces_;
	std::vector<std::vector<int> > texfaces_;
	std::vector<std::vector<int> > texfacesnormal_;

public:
	Model(const char* filename);
	~Model();
	int nverts();
	int ntexverts();
	int ntexnomal();
	int nfaces();
	int ntexfaces();
	int ntexfacenormal();
	Vec3f vert(int i);
	Vec2f texvert(int i);
	Vec3f texnormal(int i);
	std::vector<int> face(int idx);
	std::vector<int> texface(int idx);
	std::vector<int> texfacenormal(int idx);
};

#endif //__MODEL_H__
