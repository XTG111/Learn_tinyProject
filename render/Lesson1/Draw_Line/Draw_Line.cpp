#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);


//����ֱ�߷���
//��x0,y0��x1,y1��ֱ�ߣ��������ڵĵ㣬�Ե��γ�ֱ��
//����t��ֵ���Ըı�����Ч��
//void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
//    for (float t = 0.; t < 1.; t += .01) {
//        int x = x0 * (1. - t) + x1 * t;
//        int y = y0 * (1. - t) + y1 * t;
//        image.set(x, y, color);
//    }
//}

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
    //��x0-x1<y0-y1ʱ������x,y��ȷ��һ����Ĳ�����
    //ע����ж�����x0>x1�ж�֮ǰ���������Ա�֤�߶����Ǵ������һ���
    //��Ҫ����һ��Boolֵ����Ϊ�ڻ���ʱ��x,y��Ҫ��������
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
        //һ��Ҫ��y,x����������������ƵĲ���������Ҫ��ֱ��
        if (BigC) {
            image.set(y, x, color);
        }
        else {
            image.set(x, y, color);
        }
    }
}

int main() {
    //����һ��image���������洢���ƺõ�ֱ��
    TGAImage image(100, 100, TGAImage::RGB);
    line(13, 40, 20, 80, image, white);
    image.set(0, 0, red);
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}