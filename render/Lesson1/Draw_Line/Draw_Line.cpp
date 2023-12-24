#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);


//绘制直线方程
//从x0,y0到x1,y1的直线，绘制相邻的点，以点形成直线
//更改t的值可以改变走样效果
//void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
//    for (float t = 0.; t < 1.; t += .01) {
//        int x = x0 * (1. - t) + x1 * t;
//        int y = y0 * (1. - t) + y1 * t;
//        image.set(x, y, color);
//    }
//}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    //当x0-x1<y0-y1时，调换x,y，确定一个大的采样率
    //注意此判断须在x0>x1判断之前，这样可以保证线段总是从左向右绘制
    //需要定义一个Bool值，因为在绘制时，x,y又要交换回来
    bool BigC = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        BigC = true;
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    for (float x = x0; x <= x1; x++) {
        float t = (x - x0) / (float)(x1 - x0);
        int y = y0 * (1. - t) + y1 * t;
        //一定要将y,x交换回来，否则绘制的不是我们想要的直线
        if (BigC) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }
}

int main() {
    //定义一个image变量用来存储绘制好的直线
    TGAImage image(100, 100, TGAImage::RGB);
    line(13, 40, 20, 80, image, white);
    image.set(0, 0, red);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}