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

	//����ֱ��
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

//����������
void triangle(Vec2i vec1, Vec2i vec2, Vec2i vec3, TGAImage& image, TGAColor color)
{
	//��y������
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

	//�����εı߽�
	//line(vec1, vec2, image, white);
	//line(vec2, vec3, image, red);
	//line(vec3, vec1, image, green);

	//vec1 -> vec2 -> vec3��yֵ��С����
	//����������ʵ���Ϊ��������vec2��y�ֿ�����
	// 
	//������vec1 -> vec2
	//�ⲿ�ֱ�������vec1vec2ֱ�ߣ�vec1vec3ֱ�ߺ�vec��y��
	for (int y = vec1.y; y < vec2.y; y++)
	{
		//��������ֱ�ߵķ�б�� ����y��x
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


//���������������������

//��ȡ(u,v,1)������ pts ��ʾ������ ��P��ʾ��һ��
Vec3f barycentric(Vec2i* pts, Vec2i P)
{
	//�����������^��������ཻ����
	Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[1].y- pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
	//���zֵΪ0��˵����������β�����
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	//P����������
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

//��Χ����Ⱦ���ж�uv�����Ƿ���������
void triangle(Vec2i* pts, TGAImage& image, TGAColor color)
{
	//�����Χ��
	Vec2i bboxmin(image.get_width() - 1, image.get_height() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.get_width() - 1, image.get_height() - 1);
	//������������
	for (int i = 0; i < 3; i++)
	{
		//��Сx ,y����������������С��x��y
		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	//����֤�ĵ�P,����������Χ��
	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) 
		{
			//��ǰP����������
			Vec3f bc_screen = barycentric(pts, P);
			//����������ֵ�и���������
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

	////�ڶ��ַ�������һ��������
	////������������������
	//Vec2i pts[3] = { Vec2i(10,10),Vec2i(100,30),Vec2i(190,160) };
	////����
	//triangle(pts, image, red);

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