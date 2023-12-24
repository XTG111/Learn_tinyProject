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
		std::swap(vec1.u,vec1.y);
		std::swap(vec2.u, vec2.y);
		steep = true;
		//std::cout << steep << std::endl;
	}
	if (vec1.u > vec2.u)
	{
		std::swap(vec1.u, vec2.u);
		std::swap(vec1.y, vec2.y);
	}

	//绘制直线
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

//绘制三角形
void triangle(Vec2i vec1, Vec2i vec2, Vec2i vec3, TGAImage& image, TGAColor color)
{
	//以y轴排序
	if (vec1.y > vec2.y)
	{
		std::swap(vec1, vec2);
	}
	if (vec1.y > vec3.y)
	{
		std::swap(vec1, vec3);
	}
	if (vec2.y > vec3.y)
	{
		std::swap(vec2, vec3);
	}

	//三角形的边界
	//line(vec1, vec2, image, white);
	//line(vec2, vec3, image, red);
	//line(vec3, vec1, image, green);

	//vec1 -> vec2 -> vec3的y值从小到大
	//绘制三角形实体分为两部，从vec2的y分开来画
	// 
	//首先是vec1 -> vec2
	//这部分被包含在vec1vec2直线，vec1vec3直线和vec的y轴
	for (int y = vec1.y; y < vec2.y; y++)
	{
		//计算两条直线的反斜率 根据y求x
		float dyxvec1vec2 = (float)(vec1.u - vec2.u) / (vec1.y - vec2.y);
		float dxyvec1vec2 = (float)(vec1.y - vec2.y) / (vec1.u - vec2.u);
		float bvec1vec2 = (float)(vec1.y - dxyvec1vec2 * vec1.u) / dxyvec1vec2;

		float dyxvec1vec3 = (float)(vec1.u - vec3.u) / (vec1.y - vec3.y);
		float dxyvec1vec3 = (float)(vec1.y - vec3.y) / (vec1.u - vec3.u);
		float bvec1vec3 = (float)(vec1.y - dxyvec1vec3 * vec1.u) / dxyvec1vec3;
		int x0 = y * dyxvec1vec2 - bvec1vec2;
		int x1 = y * dyxvec1vec3 - bvec1vec3;
		//std::cout << bvec1vec3 << std::endl;
		if (x0 > x1)
		{
			std::swap(x0, x1);
		}
		for (int x = x0; x < x1; x++)
		{
			//std::cout <<x << std::endl;
			image.set(x, y, color);
		}
	}

	//vec2->vec3
	for (int y = vec2.y; y < vec3.y; y++)
	{
		//vec2vec3
		float dyxvec2vec3 = (float)(vec2.u - vec3.u) / (vec2.y - vec3.y);
		float dxyvec2vec3 = (float)(vec2.y - vec3.y) / (vec2.u - vec3.u);
		float bvec2vec3 = (float)(vec2.y - dxyvec2vec3 * vec2.u) / dxyvec2vec3;
		//vec1vec3
		float dyxvec1vec3 = (float)(vec1.u - vec3.u) / (vec1.y - vec3.y);
		float dxyvec1vec3 = (float)(vec1.y - vec3.y) / (vec1.u - vec3.u);
		float bvec1vec3 = (float)(vec1.y - dxyvec1vec3 * vec1.u) / dxyvec1vec3;
		int x0 = y * dyxvec2vec3 - bvec2vec3;
		int x1 = y * dyxvec1vec3 - bvec1vec3;
		//std::cout << bvec1vec3 << std::endl;
		if (x0 > x1)
		{
			std::swap(x0, x1);
		}
		for (int x = x0; x < x1; x++)
		{
			//std::cout << x << std::endl;
			image.set(x, y, color);
		}
		
	}
}

int main(int argc, char** argv)
{
	TGAImage image(500, 500, TGAImage::RGB);
	Vec2i vec1[3] = { Vec2i(10,70),Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i vec2[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i vec3[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	//line(vec1[0], vec1[1], image, red);
	//line(vec1[1], vec1[2], image, red);
	triangle(vec1[0], vec1[1], vec1[2], image, red);
	triangle(vec2[0], vec2[1], vec2[2], image, white);
	triangle(vec3[0], vec3[1], vec3[2], image, green);
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}