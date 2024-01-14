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
			errorx = errorx - 2 * dx;
		}
	}
}


int main(int argc, char** argv)
{
	TGAImage image(800, 800, TGAImage::RGB);

	//ʹ��ģ��
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head.obj");
	}

	//��Դ
	Vec3f light_Dir(0, 0, -1);
	for (int i = 0; i < model->nfaces(); i++)
	{
		//��ÿ�����Ϊ����
		std::vector<int> face = model->face(i);
		//��������ÿ�������������
		Vec2i screen_coords[3];
		//�����������������ȡ��������
		Vec3f  world_coords[3];
		for (int j = 0; j < 3; j++) {
			//ÿ������ֵ�Ļ�ȡ
			Vec3f v = model->vert(face[j]);
			//ͶӰ����Ļ
			screen_coords[j] = Vec2i((v.x + 1.) * width / 2.2, (v.y + 1.) * height / 2.2);
			world_coords[j] = v;
		}

		//��������ķ���
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		//��һ��
		n.normalize();
		//����ǿ�ȱ���
		float intensity = n * light_Dir;
		//�Բ�ͬ��ɫ����ÿ��������
		std::cout << i << std::endl;
		//if(intensity > 0)
		triangle(screen_coords, image, TGAColor(intensity*255,intensity*255,intensity*255,255));
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}