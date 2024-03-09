#include <vector>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "SelfGLFunc.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor yellow = TGAColor(255, 255, 0, 255);

Model* model = nullptr;
const int width = 800;
const int height = 800;

//光源
//Vec3f light_Dir(0, 0, -1);
Vec3f light_Dir = Vec3f(1, 1, 0).normalize();
//相机
Vec3f eye(1, 1, 4);
//原始图像中心点
Vec3f center(0, 0, 0);
//设定的up向量
Vec3f up(0, 1, 0);

//纯色GourandShader
struct  GourandShder: public IShader
{
    //记录三个顶点的光照强度
    Vec3f varing_intensity;

    //读取obj文件中三角形顶点的数据
    virtual Vec4f vertex(int iface, int nthvert)
    {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        //MVP变换
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
        varing_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_Dir);
        return gl_Vertex;
    }

    //处理着色
    virtual bool fragement(Vec3f bar, TGAColor& color)
    {
        //像素点的光强
        float internsity = varing_intensity * bar;
        color = TGAColor(255, 255, 255) * internsity;
        return false;
    }
};

//纹理映射GourandShader
struct uvShader : public IShader
{
    Vec3f varying_intensity;
    //存储的每个顶点的uv值
    mat<2, 3, float> varing_uv;

    virtual Vec4f vertex(int iface, int nthvert)
    {
        varing_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_Dir);
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        return Viewport * Projection * ModelView * gl_Vertex;
    }

    virtual bool fragement(Vec3f bar, TGAColor& color)
    {
        float intensity = varying_intensity * bar;
        Vec2f uv = varing_uv * bar;
        color = model->diffuse(uv) * intensity;
        return false;
    }
};

//法线贴图得到每个像素点的法线计算光强
struct NormalShader : public IShader
{
    mat<2, 3, float> varying_uv;
    //变换矩阵 透视*模型 用于模型点的变化
    mat<4, 4, float> uniform_M;
    //逆变换矩阵 用于求解法线的变化
    mat<4, 4, float> uniform_MIT;

    virtual Vec4f vertex(int iface, int nthvert) {
        //一个三角形中 获得每个顶点的uv
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
    }

    virtual bool fragement(Vec3f bar, TGAColor& color) {
        Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
        //每个像素点对应的uv对应的法线
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        //光线进行MVP变换后的效果
        Vec3f l = proj<3>(uniform_M * embed<4>(light_Dir)).normalize();
        //计算光强
        float intensity = std::max(0.f, n * l);
        color = model->diffuse(uv) * intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};

//添加镜面反射
struct GlossyShader : public IShader
{
    mat<2, 3, float> varying_uv;
    mat<4, 4, float> uniform_M;
    mat<4, 4, float> uniform_MIT;

    virtual Vec4f vertex(int iface, int nthvert)
    {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        return Viewport * Projection * ModelView * gl_Vertex;
    }

    virtual bool fragement(Vec3f bar, TGAColor& color)
    {
        Vec2f uv = varying_uv * bar;
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        Vec3f l = proj<3>(uniform_M * embed<4>(light_Dir)).normalize();
        //反射光线
        Vec3f r = (n * (n * l) * 2.f - l).normalize();
        //镜面反射强度
        float spec = pow(std::max(r.z, 0.f), model->specular(uv));
        //光照强度
        float diff = std::max(0.f, n * l);
        //纹理颜色
        TGAColor c = model->diffuse(uv);
        color = c;
        //着色
        for (int i = 0; i < 3; i++)
        {
            color[i] = std::min<float>(5 + c[i] * (diff + 0.6f * spec), 255);
        }
        //5 + c[i] * (1.f * diff + 0.6f * spec)
        //环境光照，1.f表示漫反射系数 0.6f表示镜面反射系数
        return false;
    }
};

//基础的Blinn-Phong
struct PhongShader : public IShader {
    mat<2, 3, float> varying_uv;  // triangle uv coordinates, written by the vertex shader, read by the fragment shader
    mat<3, 3, float> varying_nrm; // normal per vertex to be interpolated by FS

    virtual Vec4f vertex(int iface, int nthvert) {
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        varying_nrm.set_col(nthvert, proj<3>((Projection * ModelView).invert_transpose() * embed<4>(model->normal(iface, nthvert), 0.f)));
        Vec4f gl_Vertex = Projection * ModelView * embed<4>(model->vert(iface, nthvert));
        //varying_tri.set_col(nthvert, gl_Vertex);
        return gl_Vertex;
    }

    virtual bool fragment(Vec3f bar, TGAColor& color) {
        Vec3f bn = (varying_nrm * bar).normalize();
        Vec2f uv = varying_uv * bar;

        float diff = std::max(0.f, bn * light_Dir);
        color = model->diffuse(uv) * diff;
        return false;
    }
};

int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/diablo3_pose.obj");
    }

    lookat(eye, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye - center).norm());
    TGAImage image(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);
    
    //GourandShder shader;
    //uvShader shader;
    //NormalShader shader;
    GlossyShader shader;
    //用于控制法线的变换
    shader.uniform_M = Projection*ModelView;
    shader.uniform_MIT = (Projection * ModelView).invert_transpose();
    for (int i = 0; i < model->nfaces(); i++)
    {
        Vec4f screen_coords[3];
        for (int j = 0; j < 3; j++)
        {
            screen_coords[j] = shader.vertex(i, j);
        }
        triangle(screen_coords, shader, image, zbuffer);
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}