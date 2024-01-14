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
	//���н������ж�,���ߵĻ��ƶ���Ϊ������
	bool Bigc = false;
	if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
		std::swap(p0.x, p0.y);
		std::swap(p1.x, p1.y);
		Bigc = true;
	}
	//�����ж�б��ʱ��Ҫ����x
	if (p0.x > p1.x) {
		std::swap(p0.x, p1.x);
		std::swap(p0.y, p1.y);
	}

	//��������ʾ���Ƶ��ʵ��λ�ò���1/2�Ƚϣ����һ�Ҫ���⸡������
	//error+k �� 1/2�ıȽ�
	//k = dy/dx
	//2error*dx+2dy��dx�ıȽ� 
	int dx = p1.x - p0.x;
	int dy = p1.y - p0.y;
	int deerror = std::abs(dy * 2);

	//���errorx��������2error*dx
	int errorx = 0;
	int y = p0.y;
	for (int x = p0.x; x <= p1.x; x++) {
		if (Bigc) {
			//���dx��dyС����ôǰ��ͽ�����x��y����Ϊ�˱�����ƹ����в����������
			//��ʵ�ʻ��Ƶ�x,y�����ǲ��ܸı�ģ�������Ƴ�����ֱ��б�ʶ���0��1֮��
			//����image.set()��x��y��Ҫ������
			image.set(y, x, color);
		}
		else {
			image.set(x, y, color);
		}
		//����error��dx�Ĺ�ϵ��Ҫ��2*dy
		errorx += deerror;

		//���error>dx��ô��һ��y��λ����Ҫ�������ģ���б�ʾ�����+1����-1
		if (errorx > dx) {
			y += (p1.y > p0.y ? 1 : -1);
			//���ֵerror�ᷢ���ı�error+k-1����error+k+1
			//(p1.y > p0.y ? 1 : -1)*
			errorx = errorx - 2 * dx;
		}
	}
}

//�ڶ�ά������ybuffer����Ⱦ���
//��Ⱦ˼·
//�������ǳ�ʼ��ybufferΪһ�����飬�洢����x����ÿ�����y����ȣ�ԽСԽ��
//��ʼ��ybuffer[] ��Ԫ��ȫΪ��СֵINT_MIN
//Ȼ������x�ĸ���ȥ��⵱ǰx���ڵ�y
//�����yֵ��ybuffer��Ӧ��ֵ����ô˵�����������������Ҳ��˵������Ա�����
//ͬʱ����Ҫ����ybuffer��ֵΪ��ǰ��y
void rasterize2D(Vec2i p0, Vec2i p1, TGAImage& image, const TGAColor& color, int ybuffer[])
{
	//��֤��С����
	if (p0.x > p1.x)
	{
		std::swap(p0, p1);
	}

	//��x0��ʼ��x1����ybuffer�����Ƶ�
	for (int x = p0.x; x <= p1.x; x++)
	{
		//���ñ�����ȡx��Ӧ��y
		float t = (x - p0.x) / float(p1.x - p0.x);
		int y = p0.y * (1 - t) + p1.y * t;
		//����ybuffer������
		if (ybuffer[x] < y)
		{
			ybuffer[x] = y;
			//������Ⱦ��Ϊ1ά������y=0
			image.set(x, 0, color);
		}
	}
}


int main(int argc, char** argv)
{
	TGAImage image(800, 1, TGAImage::RGB);
	
	//ybuffer����
	int ybuffer[width];
	for (int i = 0; i < width; i++)
	{
		ybuffer[i] = std::numeric_limits<int>::min();
	}

	//�·����������λ���
	rasterize2D(Vec2i(20, 34), Vec2i(744, 400), image, red, ybuffer);
	rasterize2D(Vec2i(120, 434), Vec2i(444, 400), image, green, ybuffer);
	rasterize2D(Vec2i(330, 463), Vec2i(594, 200), image, blue, ybuffer);

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}