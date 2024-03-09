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

//��Դ
//Vec3f light_Dir(0, 0, -1);
Vec3f light_Dir = Vec3f(1, 1, 0).normalize();
//���
Vec3f eye(1, 1, 4);
//ԭʼͼ�����ĵ�
Vec3f center(0, 0, 0);
//�趨��up����
Vec3f up(0, 1, 0);

//��ɫGourandShader
struct  GourandShder: public IShader
{
    //��¼��������Ĺ���ǿ��
    Vec3f varing_intensity;

    //��ȡobj�ļ��������ζ��������
    virtual Vec4f vertex(int iface, int nthvert)
    {
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert));
        //MVP�任
        gl_Vertex = Viewport * Projection * ModelView * gl_Vertex;
        varing_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert) * light_Dir);
        return gl_Vertex;
    }

    //������ɫ
    virtual bool fragement(Vec3f bar, TGAColor& color)
    {
        //���ص�Ĺ�ǿ
        float internsity = varing_intensity * bar;
        color = TGAColor(255, 255, 255) * internsity;
        return false;
    }
};

//����ӳ��GourandShader
struct uvShader : public IShader
{
    Vec3f varying_intensity;
    //�洢��ÿ�������uvֵ
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

//������ͼ�õ�ÿ�����ص�ķ��߼����ǿ
struct NormalShader : public IShader
{
    mat<2, 3, float> varying_uv;
    //�任���� ͸��*ģ�� ����ģ�͵�ı仯
    mat<4, 4, float> uniform_M;
    //��任���� ������ⷨ�ߵı仯
    mat<4, 4, float> uniform_MIT;

    virtual Vec4f vertex(int iface, int nthvert) {
        //һ���������� ���ÿ�������uv
        varying_uv.set_col(nthvert, model->uv(iface, nthvert));
        Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
        return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
    }

    virtual bool fragement(Vec3f bar, TGAColor& color) {
        Vec2f uv = varying_uv * bar;                 // interpolate uv for the current pixel
        //ÿ�����ص��Ӧ��uv��Ӧ�ķ���
        Vec3f n = proj<3>(uniform_MIT * embed<4>(model->normal(uv))).normalize();
        //���߽���MVP�任���Ч��
        Vec3f l = proj<3>(uniform_M * embed<4>(light_Dir)).normalize();
        //�����ǿ
        float intensity = std::max(0.f, n * l);
        color = model->diffuse(uv) * intensity;      // well duh
        return false;                              // no, we do not discard this pixel
    }
};

//��Ӿ��淴��
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
        //�������
        Vec3f r = (n * (n * l) * 2.f - l).normalize();
        //���淴��ǿ��
        float spec = pow(std::max(r.z, 0.f), model->specular(uv));
        //����ǿ��
        float diff = std::max(0.f, n * l);
        //������ɫ
        TGAColor c = model->diffuse(uv);
        color = c;
        //��ɫ
        for (int i = 0; i < 3; i++)
        {
            color[i] = std::min<float>(5 + c[i] * (diff + 0.6f * spec), 255);
        }
        //5 + c[i] * (1.f * diff + 0.6f * spec)
        //�������գ�1.f��ʾ������ϵ�� 0.6f��ʾ���淴��ϵ��
        return false;
    }
};

//������Blinn-Phong
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
    //���ڿ��Ʒ��ߵı任
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