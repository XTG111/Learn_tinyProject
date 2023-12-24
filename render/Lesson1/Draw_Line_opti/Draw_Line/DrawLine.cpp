#include "tgaimage.h"
#include <iostream>
#include <cmath>

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color);

int main() {
	TGAImage image(100, 100, TGAImage::RGB);
	for (int i = 0; i < 1000000; i++) {
		line(13, 20, 80, 40, image, red);
		line(13, 40, 80, 20, image, red);
		line(80, 40, 13, 20, image, red);
	}
	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color)
{
	//���н������ж�,���ߵĻ��ƶ���Ϊ������
	bool Bigc = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		Bigc = true;
	}
	//�����ж�б��ʱ��Ҫ����x
	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	//��������ʾ���Ƶ��ʵ��λ�ò���1/2�Ƚϣ����һ�Ҫ���⸡������
	//error+k �� 1/2�ıȽ�
	//k = dy/dx
	//2error*dx+2dy��dx�ıȽ� 
	int dx = x1 - x0;
	int dy = y1 - y0;
	int deerror = std::abs(dy * 2);

	//���errorx��������2error*dx
	int errorx = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++) {
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
			y += (y1 > y0 ? 1 : -1);
			//���ֵerror�ᷢ���ı�error+k-1����error+k+1
			//(y1 > y0 ? 1 : -1)*
			errorx = errorx -  2 * dx;
		}
	}
}
