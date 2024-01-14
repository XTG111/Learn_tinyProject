#include "geometry.h"
#include "model.h"
#include "tgaimage.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

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


//利用重心坐标求解三角形

//获取(u,v,1)的坐标 pts 表示三角形 ，P表示那一点
Vec3f barycentric(Vec2i* pts, Vec2i P)
{
	//两个向量叉乘^即可求得相交向量
	Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[1].y- pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
	//如果z值为0，说明这个三角形不存在
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	//P的重心坐标
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

//包围盒渲染，判断uv坐标是否满足条件
void triangle(Vec2i* pts, TGAImage& image, TGAColor color)
{
	//定义包围盒
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//遍历三个顶点
	for (int i = 0; i < 3; i++)
	{
		//最小x ,y就是三个顶点中最小的x和y
		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	//待求证的点P,遍历整个包围和
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) 
		{
			//当前P的重心坐标
			Vec3f bc_screen = barycentric(pts, P);
			//当三个坐标值有负数就跳过
			if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
			{
				continue;
			}
			image.set(P.x, P.y, color);
		}
	}
}

int main(int argc, char** argv)
{
	TGAImage image(800, 800, TGAImage::RGB);
	//Vec2i vec1[3] = { Vec2i(10,70),Vec2i(50, 160),  Vec2i(70, 80) };
	//Vec2i vec2[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	//Vec2i vec3[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };
	////line(vec1[0], vec1[1], image, red);
	////line(vec1[1], vec1[2], image, red);
	//triangle(vec1[0], vec1[1], vec1[2], image, red);
	//triangle(vec2[0], vec2[1], vec2[2], image, white);
	//triangle(vec3[0], vec3[1], vec3[2], image, green);

	////第二种方法绘制一个三角形
	////定义三角形三个顶点
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };
	////绘制
	//triangle(pts, image, red);

	//使用模型
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj");
	}

	//光源
	Vec3f light_Dir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++)
	{
		//将每个面存为数组
		std::vector<int> face = model->face(i);
		//用来接受每个面的三个顶点
		Vec2i screen_coords[3];
		//用来存放三个顶点求取法线向量
		Vec3f  world_coords[3];
		for (int j = 0; j < 3; j++) {
			//每个顶点值的获取
			Vec3f v = model->vert(face[j]);
			//投影到屏幕
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2.2, (v.y + 1.) * height / 2.2);
			world_coords[j] = v;
		}

		//三角形面的法线
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		//归一化
		n.normalize();
		//光照强度比例
		float intensity = n * light_Dir;
		//以不同颜色绘制每个三角形
		std::cout << i << std::endl;
		//if(intensity > 0)
		triangle(screen_coords, image, TGAColor(intensity*255,intensity*255,intensity*255,255));
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}