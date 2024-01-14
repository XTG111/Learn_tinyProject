#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
#include <iostream>
#include <vector>
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(60, 179, 113, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

void line(Vec2i vec1, Vec2i vec2, TGAImage& image, TGAColor color)
{
	bool steep = false;
	if (std::abs(vec1.u - vec2.u) < std::abs(vec1.y - vec2.y))
	{
		std::swap(vec1.u, vec1.y);
		std::swap(vec2.u, vec2.y);
		steep = true;
		//std::cout << steep << std::endl;
	}
	if (vec1.u > vec2.u)
	{
		std::swap(vec1.u, vec2.u);
		std::swap(vec1.y, vec2.y);
	}

	//ªÊ÷∆÷±œﬂ
	int dx = vec1.u - vec2.u;
	int dy = vec1.y - vec2.y;

	int deerror = std::abs(2 * dy);
	int errordx = 0;
	int y = vec1.y;

	for (int x = vec1.u; x < vec2.u; x++)
	{
		if (!steep)
		{
			image.set(x, y, color);
		}
		else
		{
			image.set(y, x, color);
		}
		//std::cout << " "<<  x << " " << y << std::endl;
		errordx += deerror;
		if (errordx > dx)
		{
			y += (vec1.y > vec2.y ? -1 : 1);
			errordx -= 2 * std::abs(dx);
		}
	}
}
