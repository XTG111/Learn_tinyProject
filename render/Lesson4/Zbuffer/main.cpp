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
const TGAColor blue = TGAColor(0, 0, 255, 255);
Model* model = NULL;
const int width = 500;
const int height = 500;

//利用重心坐标求解三角形

//获取(u,v,1)的坐标 pts 表示三角形 ，P表示那一点
Vec3f barycentric(Vec3f* pts, Vec3f P)
{
	//两个向量叉乘^即可求得相交向量
	Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[1].y- pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
	//如果z值为0，说明这个三角形不存在
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	//P的重心坐标
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}


void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color)
{

	//定义包围盒
	//定义包围盒
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	//遍历三个顶点
	for (int i = 0; i < 3; i++)
	{
		//最小x ,y就是三个顶点中最小的x和y
		bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	//待求证的点P,遍历整个包围和
	Vec3f P;
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
			//zbuffer
			//此时的P点即我们要绘制的点，现在需要求取z值
			P.z = 0;
			P.z = (pts[0] * bc_screen.x + pts[1] * bc_screen.y + pts[2] * bc_screen.z).z;

			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.set(P.x, P.y, color);
			}

		}
	}
}

//包围盒渲染，判断uv坐标是否满足条件
void triangle(Vec3f* pts,Vec2i* uv, float *zbuffer, TGAImage& image, float intensity, TGAImage& TextureImg)
{

	//定义包围盒
	//定义包围盒
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	//遍历三个顶点
	for (int i = 0; i < 3; i++)
	{
		//最小x ,y就是三个顶点中最小的x和y
		bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	//待求证的点P,遍历整个包围和
	Vec3f P;
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
			//zbuffer
			//此时的P点即我们要绘制的点，现在需要求取z值
			P.z = 0;
			P.z = (pts[0] * bc_screen.x+ pts[1] * bc_screen.y+ pts[2] * bc_screen.z).z;

			//当前点的纹理坐标
			Vec2i uvp = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;

			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				zbuffer[int(P.x + P.y * width)] = P.z;
				TGAColor pcolor = TextureImg.get(uvp.x,uvp.y);
				image.set(P.x, P.y, TGAColor(pcolor.r * intensity,pcolor.g*intensity,pcolor.b*intensity,255));
			}

		}
	}
}

int main(int argc, char** argv)
{
	TGAImage image(width, height, TGAImage::RGB);
	//使用模型
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head_new.obj");
	}
	
	TGAImage TextureImg;
	TextureImg.read_tga_file("obj/african_head_diffuse.tga");
	TextureImg.flip_vertically();


	//光源
	Vec3f light_Dir(0,0,-1);
	float zbuffer[width*height];
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = -std::numeric_limits<float>::max();
	}
	for (int i = 0; i < model->nfaces(); i++)
	{
		//将每个面存为数组
		std::vector<int> face = model->face(i);
		std::vector<float> texface = model->texface(i);
		//用来接受每个面的三个顶点
		Vec3f screen_coords[3];
		//用来存放三个顶点求取法线向量
		Vec3f  world_coords[3];
		//存放三个顶点的纹理坐标
		Vec2i uvp[3];

		for (int j = 0; j < 3; j++) {
			//每个顶点值的获取
			Vec3f v = model->vert(face[j]);

			//每个顶点的纹理坐标
			Vec2f tv = model->texvert(texface[j]);
			//投影到屏幕
			//需要对x,y值取整，因为像素值
			screen_coords[j] = Vec3f(int((v.x + 1.) * width / 2.), int((v.y + 1.) * height / 2.), v.z);
			uvp[j] = Vec2i(tv.x * TextureImg.get_width(), tv.y * TextureImg.get_height());
			world_coords[j] = v;
			std::cout << tv.x * TextureImg.get_width() << std::endl;
		}

		//三角形面的法线
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		//归一化
		n.normalize();
		//光照强度比例
		float intensity = n * light_Dir;
		//以不同颜色绘制每个三角形
		
		if(intensity > 0)
		//triangle(screen_coords, zbuffer, image, TGAColor(255*intensity, 255*intensity, 255*intensity,255));
		triangle(screen_coords, uvp, zbuffer, image, intensity, TextureImg);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}