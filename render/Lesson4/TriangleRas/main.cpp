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

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	//进行交换，判断,将线的绘制都变为从左到右
	bool Bigc = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		Bigc = true;
	}
	//避免判断斜率时还要考虑x
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	//利用误差表示绘制点的实际位置并与1/2比较，并且还要避免浮点运算
	//error+k 与 1/2的比较
	//k = dy/dx
	//2error*dx+2dy与dx的比较 
	int dx = x1 - x0;
	int dy = y1 - y0;
	int deerror = std::abs(dy * 2);

	//这个errorx不是误差，是2error*dx
	int errorx = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
		if (Bigc) {
			//如果dx比dy小，那么前面就交换了x和y，是为了避免绘制过程中产生点的拉扯
			//但实际绘制的x,y坐标是不能改变的，否则绘制出来的直线斜率都在0，1之间
			//所以image.set()中x和y需要交换。
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		//不论error与dx的关系都要加2*dy
		errorx += deerror;

		//如果error>dx那么下一个y的位置需要发生更改，由斜率决定是+1还是-1
		if (errorx > dx) {
			y += (y1 > y0 ? 1 : -1);
			//误差值error会发生改变error+k-1或者error+k+1
			//(y1 > y0 ? 1 : -1)*
			errorx = errorx - 2 * dx;
		}
	}
}


int main(int argc, char** argv)
{
	TGAImage image(800, 800, TGAImage::RGB);

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