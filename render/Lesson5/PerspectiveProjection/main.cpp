#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);
const TGAColor yellow = TGAColor(255, 255, 0, 255);

//光源
Vec3f light_Dir(0, 0, -1);
//相机
Vec3f cameraPos(0, 0, 3);

Model* model = NULL;
const int width = 500;
const int height = 500;
const int depth = 255;

//绘制直线
void line(Vec3i p0, Vec3i p1, TGAImage& image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }

    for (int x = p0.x; x <= p1.x; x++) {
        float t = (x - p0.x) / (float)(p1.x - p0.x);
        int y = p0.y * (1. - t) + p1.y * t + .5;
        if (steep) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }
}

//利用重心坐标求解三角形

//获取(u,v,1)的坐标 pts 表示三角形 ，P表示那一点
Vec3f barycentric(Vec3f* pts, Vec3f P)
{
    //两个向量叉乘^即可求得相交向量
    Vec3f u = Vec3f(pts[1].x - pts[0].x, pts[2].x - pts[0].x, pts[0].x - P.x) ^ Vec3f(pts[1].y - pts[0].y, pts[2].y - pts[0].y, pts[0].y - P.y);
    //如果z值为0，说明这个三角形不存在
    if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
    //P的重心坐标
    return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}


void triangle(Vec3f* pts, float* zbuffer, TGAImage& image, TGAColor color)
{

    //定义包围盒
    //定义包围盒
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    //遍历三个顶点
    for (int i = 0; i < 3; i++)
    {
        //最小x ,y就是三个顶点中最小的x和y
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    //待求证的点P,遍历整个包围和
    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            //当前P的重心坐标
            Vec3f bc_screen = barycentric(pts, P);
            //当三个坐标值有负数就跳过
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
            {
                continue;
            }
            //zbuffer
            //此时的P点即我们要绘制的点，现在需要求取z值
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

//包围盒渲染，判断uv坐标是否满足条件
void triangle(Vec3f* pts, Vec2i* uv, float* zbuffer, TGAImage& image, float intensity, TGAImage& TextureImg)
{

    //定义包围盒
    //定义包围盒
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    //遍历三个顶点
    for (int i = 0; i < 3; i++)
    {
        //最小x ,y就是三个顶点中最小的x和y
        bboxmin.x = std::max(0.f, std::min(bboxmin.x, pts[i].x));
        bboxmin.y = std::max(0.f, std::min(bboxmin.y, pts[i].y));

        bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
        bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
    }
    //待求证的点P,遍历整个包围和
    Vec3i P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            //当前P的重心坐标
            Vec3f bc_screen = barycentric(pts, P);
            //当三个坐标值有负数就跳过
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0)
            {
                continue;
            }
            //zbuffer
            //此时的P点即我们要绘制的点，现在需要求取z值
            P.z = 0;
            P.z = (pts[0] * bc_screen.x + pts[1] * bc_screen.y + pts[2] * bc_screen.z).z;

            //当前点的纹理坐标
            Vec2i uvp = uv[0] * bc_screen.x + uv[1] * bc_screen.y + uv[2] * bc_screen.z;

            if (zbuffer[int(P.x + P.y * width)] < P.z)
            {
                zbuffer[int(P.x + P.y * width)] = P.z;
                TGAColor pcolor = TextureImg.get(uvp.x, uvp.y);
                image.set(P.x, P.y, TGAColor(pcolor.r * intensity, pcolor.g * intensity, pcolor.b * intensity, 255));
            }

        }
    }
}


//将(x,y,z,k) -> (x/k,y/k,z/k,1) 齐次转换为三维正确坐标
Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}
//转换为齐次坐标
Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

//模型变换M
Matrix modelMatrix()
{
    return Matrix::identity(4);
}

//视图变换
Matrix ViewMatrix()
{
    return Matrix::identity(4);
}

//投影变换
Matrix projectionMatrix()
{
    Matrix projection = Matrix::identity(4);
    projection[3][2] = -1.f / cameraPos.z;
    return projection;
}

//变为屏幕坐标
Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

//平移变化矩阵
Matrix translation(Vec3f v) {
    Matrix Tr = Matrix::identity(4);
    Tr[0][3] = v.x;
    Tr[1][3] = v.y;
    Tr[2][3] = v.z;
    return Tr;
}

//缩放
Matrix zoom(float factor) {
    Matrix Z = Matrix::identity(4);
    Z[0][0] = Z[1][1] = Z[2][2] = factor;
    return Z;
}

//绕x轴旋转
Matrix rotation_x(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[1][1] = R[2][2] = cosangle;
    R[1][2] = -sinangle;
    R[2][1] = sinangle;
    return R;
}

//绕y轴旋转
Matrix rotation_y(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[0][0] = R[2][2] = cosangle;
    R[0][2] = sinangle;
    R[2][0] = -sinangle;
    return R;
}

//绕z轴旋转
Matrix rotation_z(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[0][0] = R[1][1] = cosangle;
    R[0][1] = -sinangle;
    R[1][0] = sinangle;
    return R;
}



int main(int argc, char** argv) {
    if (2 == argc) {
        model = new Model(argv[1]);
    }
    else {
        model = new Model("obj/african_head.obj");
    }

    //读取obj文件
    TGAImage TextureImg;
    TextureImg.read_tga_file("obj/african_head_diffuse.tga");
    TextureImg.flip_vertically();

    TGAImage image(width, height, TGAImage::RGB);
    Matrix VP = viewport(width / 8, width / 8, width *3/ 4, height *3/ 4);
    Matrix PR = projectionMatrix();
    Matrix MO = modelMatrix();
    Matrix VM = ViewMatrix();



    float zbuffer[width * height];
    for (int i = 0; i < width * height; i++)
    {
        zbuffer[i] = -std::numeric_limits<float>::max();
    }
    for (int i = 0; i < model->nfaces(); i++)
    {
        //将每个面存为数组
        std::vector<int> face = model->face(i);
        std::vector<float> texface = model->texface(i);
        //用来接受每个面的三个顶点
        Vec3f screen_coords[3];
        //用来存放三个顶点求取法线向量
        Vec3f  world_coords[3];
        //存放三个顶点的纹理坐标
        Vec2i uvp[3];

        for (int j = 0; j < 3; j++) {
            //每个顶点值的获取
            Vec3f v = model->vert(face[j]);

            //每个顶点的纹理坐标
            Vec2f tv = model->texvert(texface[j]);
            //投影到屏幕
            //需要对x,y值取整，因为像素值

            //为了透视投影，需要对screen_coordes[j]这个点做一些变换 M->V->P
            //模型 视口 投影

            //1. 转换为齐次坐标 v2m(Vec3f v)
            //2. 模型变换 将模型摆放到合理位置，不需要操作，obj文件已经满足 modelMatrix()
            //3. 视口变换 将立方体变为1*1*1已经是了，不需要操作 viewMatrix*()
            //4. 投影变换 透视投影 projectionMatrix()
            //5. 转换为屏幕坐标  viewport(int x, int y, int w, int h)
            //6. 变回一个正常点 就是除以第4个量 m2v(Matrix m)
            screen_coords[j] = Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);

            //screen_coords[j] =  m2v(VP * PR * VM * MO * v2m(v));
            uvp[j] = Vec2i(tv.x * TextureImg.get_width(), tv.y * TextureImg.get_height());
            world_coords[j] = v;
            std::cout << tv.x * TextureImg.get_width() << std::endl;
        }

        //三角形面的法线
        Vec3f n = (world_coords[2] - world_coords[0]) ^ (world_coords[1] - world_coords[0]);
        //归一化
        n.normalize();
        //光照强度比例
        float intensity = n * light_Dir;
        //以不同颜色绘制每个三角形

        if (intensity > 0)
            //triangle(screen_coords, zbuffer, image, TGAColor(255*intensity, 255*intensity, 255*intensity,255));
            triangle(screen_coords, uvp, zbuffer, image, intensity, TextureImg);
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}