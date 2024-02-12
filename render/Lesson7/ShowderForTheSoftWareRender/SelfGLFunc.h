#ifndef __SELFGLFUNC_H__
#define __SELFGLFUNC_H__
#include "geometry.h"
#include "tgaimage.h"

extern Matrix ModelView;
extern Matrix Viewport;
extern Matrix Projection;

//转换为屏幕坐标
void viewport(int x, int y, int w, int h);
//投影变换
void projection(float coeff = 0.f);//coeff = -1/c
//视角变换
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
    virtual ~IShader();
    //计算变换矩阵
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    //计算着色
    virtual bool fragement(Vec3f bar, TGAColor& color) = 0;
};

//计算重心坐标
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P);
//着色
void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);

//绘制直线
void line(Vec3i p0, Vec3i p1, TGAImage& image, TGAColor color);

#endif // !__SELFGLFUNC_H__