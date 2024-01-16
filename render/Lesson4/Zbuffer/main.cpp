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

//���������������������

//��ȡ(u,v,1)������ pts ��ʾ������ ��P��ʾ��һ��
Vec3f barycentric(Vec3f* pts, Vec3f P)
{
	//�����������^��������ཻ����
	Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[1].y- pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
	//���zֵΪ0��˵����������β�����
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	//P����������
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}


void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color)
{

	//�����Χ��
	//�����Χ��
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	//������������
	for (int i = 0; i < 3; i++)
	{
		//��Сx ,y����������������С��x��y
		bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	//����֤�ĵ�P,����������Χ��
	Vec3f P;
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
			//zbuffer
			//��ʱ��P�㼴����Ҫ���Ƶĵ㣬������Ҫ��ȡzֵ
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

//��Χ����Ⱦ���ж�uv�����Ƿ���������
void triangle(Vec3f* pts,Vec2i* uv, float *zbuffer, TGAImage& image, float intensity, TGAImage& TextureImg)
{

	//�����Χ��
	//�����Χ��
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
	//������������
	for (int i = 0; i < 3; i++)
	{
		//��Сx ,y����������������С��x��y
		bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}
	//����֤�ĵ�P,����������Χ��
	Vec3f P;
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
			//zbuffer
			//��ʱ��P�㼴����Ҫ���Ƶĵ㣬������Ҫ��ȡzֵ
			P.z = 0;
			P.z = (pts[0] * bc_screen.x+ pts[1] * bc_screen.y+ pts[2] * bc_screen.z).z;

			//��ǰ�����������
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
	//ʹ��ģ��
	if (2 == argc) {
		model = new Model(argv[1]);
	}
	else {
		model = new Model("obj/african_head_new.obj");
	}
	
	TGAImage TextureImg;
	TextureImg.read_tga_file("obj/african_head_diffuse.tga");
	TextureImg.flip_vertically();


	//��Դ
	Vec3f light_Dir(0,0,-1);
	float zbuffer[width*height];
	for (int i = 0; i < width * height; i++)
	{
		zbuffer[i] = -std::numeric_limits<float>::max();
	}
	for (int i = 0; i < model->nfaces(); i++)
	{
		//��ÿ�����Ϊ����
		std::vector<int> face = model->face(i);
		std::vector<float> texface = model->texface(i);
		//��������ÿ�������������
		Vec3f screen_coords[3];
		//�����������������ȡ��������
		Vec3f  world_coords[3];
		//��������������������
		Vec2i uvp[3];

		for (int j = 0; j < 3; j++) {
			//ÿ������ֵ�Ļ�ȡ
			Vec3f v = model->vert(face[j]);

			//ÿ���������������
			Vec2f tv = model->texvert(texface[j]);
			//ͶӰ����Ļ
			//��Ҫ��x,yֵȡ������Ϊ����ֵ
			screen_coords[j] = Vec3f(int((v.x + 1.) * width / 2.), int((v.y + 1.) * height / 2.), v.z);
			uvp[j] = Vec2i(tv.x * TextureImg.get_width(), tv.y * TextureImg.get_height());
			world_coords[j] = v;
			std::cout << tv.x * TextureImg.get_width() << std::endl;
		}

		//��������ķ���
		Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
		//��һ��
		n.normalize();
		//����ǿ�ȱ���
		float intensity = n * light_Dir;
		//�Բ�ͬ��ɫ����ÿ��������
		
		if(intensity > 0)
		//triangle(screen_coords, zbuffer, image, TGAColor(255*intensity, 255*intensity, 255*intensity,255));
		triangle(screen_coords, uvp, zbuffer, image, intensity, TextureImg);
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}