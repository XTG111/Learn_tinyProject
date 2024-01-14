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
const int width = 800;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage& image, TGAColor color)
{
	//进行交换，判断,将线的绘制都变为从左到右
	bool Bigc = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		Bigc = true;
	}
	//避免判断斜率时还要考虑x
	if (p0.x > p1.x) {
		std::swap(p0.x, p1.x);
		std::swap(p0.y, p1.y);
	}

	//利用误差表示绘制点的实际位置并与1/2比较，并且还要避免浮点运算
	//error+k 与 1/2的比较
	//k = dy/dx
	//2error*dx+2dy与dx的比较 
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int deerror = std::abs(dy * 2);

	//这个errorx不是误差，是2error*dx
	int errorx = 0;
	int y = p0.y;
	for (int x = p0.x; x <= p1.x; x++) {
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
			y += (p1.y > p0.y ? 1 : -1);
			//误差值error会发生改变error+k-1或者error+k+1
			//(p1.y > p0.y ? 1 : -1)*
			errorx = errorx - 2 * dx;
		}
	}
}

//在二维上利用ybuffer来渲染深度
//渲染思路
//首先我们初始化ybuffer为一个数组，存储的是x轴上每个点的y轴深度，越小越深
//初始化ybuffer[] 中元素全为最小值INT_MIN
//然后利用x的更新去求解当前x对于的y
//如果该y值比ybuffer对应的值大，那么说明其离摄像机更近，也就说明其可以被看到
//同时还需要更新ybuffer的值为当前的y
void rasterize2D(Vec2i p0, Vec2i p1, TGAImage& image, const TGAColor& color, int ybuffer[])
{
	//保证从小到大
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}

	//从x0开始到x1更新ybuffer并绘制点
	for (int x = p0.x; x <= p1.x; x++)
	{
		//利用比例求取x对应的y
		float t = (x - p0.x) / float(p1.x - p0.x);
		int y = p0.y * (1 - t) + p1.y * t;
		//更新ybuffer并绘制
		if (ybuffer[x] < y)
		{
			ybuffer[x] = y;
			//由于渲染后为1维，所以y=0
			image.set(x, 0, color);
		}
	}
}


int main(int argc, char** argv)
{
	TGAImage image(800, 1, TGAImage::RGB);
	
	//ybuffer数组
	int ybuffer[width];
	for (int i = 0; i < width; i++)
	{
		ybuffer[i] = std::numeric_limits<int>::min();
	}

	//下方三条线依次绘制
	rasterize2D(Vec2i(20, 34), Vec2i(744, 400), image, red, ybuffer);
	rasterize2D(Vec2i(120, 434), Vec2i(444, 400), image, green, ybuffer);
	rasterize2D(Vec2i(330, 463), Vec2i(594, 200), image, blue, ybuffer);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}