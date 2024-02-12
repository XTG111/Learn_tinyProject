#ifndef __SELFGLFUNC_H__
#define __SELFGLFUNC_H__
#include "geometry.h"
#include "tgaimage.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

//ת��Ϊ��Ļ����
void viewport(int x, int y, int w, int h);
//ͶӰ�任
void projection(float coeff = 0.f);//coeff = -1/c
//�ӽǱ任
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
    virtual ~IShader();
    //����任����
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    //������ɫ
    virtual bool fragement(Vec3f bar, TGAColor& color) = 0;
};

//������������
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P);
//��ɫ
void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);

//����ֱ��
void line(Vec3i p0, Vec3i p1, TGAImage& image, TGAColor color);

#endif // !__SELFGLFUNC_H__