#include <cmath>
#include <limits>
#include <cstdlib>
#include "SelfGLFunc.h"


Matrix ModelView;
Matrix Viewport;
Matrix Projection;

void viewport(int x, int y, int w, int h)
{
    Viewport = Matrix::identity();
    Viewport[0][3] = x + w / 2.f;
    Viewport[1][3] = y + h / 2.f;
    Viewport[2][3] = 255.f / 2.f;

    Viewport[0][0] = w / 2.f;
    Viewport[1][1] = h / 2.f;
    Viewport[2][2] = 255.f / 2.f;
}

void projection(float coeff)
{
    Projection = Matrix::identity();
    Projection[3][2] = coeff;
}

void lookat(Vec3f eye, Vec3f center, Vec3f up)
{
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    //变换矩阵
    ModelView = Matrix::identity();
    for (int i = 0; i < 3; i++)
    {
        ModelView[0][i] = x[i];
        ModelView[1][i] = y[i];
        ModelView[2][i] = z[i];
        ModelView[i][3] = -1.f * center[i];
    }
}

Vec3f barycentric(Vec2f A, Vec2f B,Vec2f C, Vec2f P)
{
    Vec3f s[2];
    for (int i = 2; i--; ) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2]) > 1e-2) 
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1);
}

//pts为MVP变换之后的三角形三个顶点
void triangle(Vec4f* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer)
{
    //定义包围盒
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width() - 1, image.get_height() - 1);
    //遍历三个顶点
    for (int i = 0; i < 3; i++)
    {
        //最小x ,y就是三个顶点中最小的x和y
        for (int j = 0; j < 2; j++)
        {
            bboxmin[j] = std::min(bboxmin[j], pts[i][j] / pts[i][3]);
            bboxmax[j] = std::max(bboxmax[j], pts[i][j] / pts[i][3]);
        }
    }
    //待求证的点P,遍历整个包围和
    Vec2i P;
    TGAColor color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
    {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
        {
            //当前P的重心坐标
            Vec3f bc_screen = barycentric(proj<2>(pts[0] / pts[0][3]), proj<2>(pts[1] / pts[1][3]), proj<2>(pts[2] / pts[2][3]), proj<2>(P));
            //计算zbuffer
            float z = pts[0][2] * bc_screen.x + pts[1][2] * bc_screen.y + pts[2][2] * bc_screen.z;
            float w = pts[0][3] * bc_screen.x + pts[1][3] * bc_screen.y + pts[2][3] * bc_screen.z;
            int frag_depth = std::max(0, std::min(255, int(z / w + 0.5f)));
            //当三个坐标值有负数就跳过
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0 || zbuffer.get(P.x,P.y)[0]>frag_depth)
            {
                continue;
            }
            //
            bool discard = shader.fragement(bc_screen, color);
            if (!discard)
            {
                zbuffer.set(P.x, P.y, TGAColor(frag_depth));
                image.set(P.x, P.y, color);
            }
        }
    }
}

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

IShader::~IShader()
{
}
