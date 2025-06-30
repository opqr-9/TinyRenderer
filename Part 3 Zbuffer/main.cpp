#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0-x1)<std::abs(y0-y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0>x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    for (int x=x0; x<=x1; x++) {
        float t = (x-x0)/(float)(x1-x0);
        int y = y0*(1.-t) + y1*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}


//第二种方法,使用包围盒包围三角形，对点逐个判断是否在三角形内(带z轴版)
Vec3f barycentric(Vec3f* pts, Vec3f p) 
{
    Vec3f AB=pts[1]-pts[0];
    Vec3f AC=pts[2]-pts[0];
    Vec3f PA=pts[0]-p;
    Vec3f tmpx=Vec3f(AB.x,AC.x,PA.x);
    Vec3f tmpy=Vec3f(AB.y,AC.y,PA.y);
    Vec3f uvLocation=cross(tmpx,tmpy);//叉乘
    if(abs(uvLocation.z)>0.01)//判断三点是否共线,true为不共线,false为共线,解释在备注中
    {
        //等同于return Vec3f(1-u-v,u,v); 
        return Vec3f(1-(uvLocation.x+uvLocation.y)/uvLocation.z,uvLocation.x/uvLocation.z,uvLocation.y/uvLocation.z);
    }
    else
    {
        return Vec3f(-1,1,1);
    }
}

void triangle(Vec3f* pts,float *zbuffer,TGAImage &image, TGAColor color)
{
    Vec2f boxmax(pts[0].x,pts[0].y);
    Vec2f boxmin(pts[0].x,pts[0].y);
    for(int i=1;i<3;i++)
    {
        boxmax.x=std::max(pts[i].x,boxmax.x);
        boxmax.y=std::max(pts[i].y,boxmax.y);
        boxmin.x=std::min(pts[i].x,boxmin.x);
        boxmin.y=std::min(pts[i].y,boxmin.y);
    }
    //避免超出图片范围
    boxmax.x=boxmax.x<width-1?boxmax.x:width-1;
    boxmax.y=boxmax.y<height-1?boxmax.y:height-1;
    boxmin.x=boxmin.x>0?boxmin.x:0;
    boxmin.y=boxmin.y>0?boxmin.y:0;
    Vec3f tmp;
    for(tmp.x=boxmin.x;tmp.x<=boxmax.x;(tmp.x)++)
    {
        for(tmp.y=boxmin.y;tmp.y<=boxmax.y;(tmp.y)++)
        {
            Vec3f uv=barycentric(pts,tmp);
            if(uv.x>=0 && uv.y>=0 && uv.z>=0)
            {
                tmp.z=0;
                for(int i=0;i<3;i++)
                {
                    tmp.z+=uv[i]*pts[i][2];
                }
                if(tmp.z>zbuffer[(int)(tmp.x+tmp.y*width)])
                {
                    zbuffer[(int)(tmp.x+tmp.y*width)]=tmp.z;
                    image.set(tmp.x, tmp.y, color);
                }
            }
        }
    }
}


// void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
//     Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
//     Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
//     Vec2f clamp(image.get_width()-1, image.get_height()-1);
//     for (int i=0; i<3; i++) std::cout<<pts[i].z<<std::endl;
//     for (int i=0; i<3; i++) {
//         for (int j=0; j<2; j++) {
//             bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
//             bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
//         }
//     }
//     Vec3f P;
//     for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
//         for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
//             Vec3f bc_screen  = barycentric(pts, P);
//             if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;
//             P.z = 0;
//             for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
//             if (zbuffer[int(P.x+P.y*width)]<P.z) {
//                 zbuffer[int(P.x+P.y*width)] = P.z;
//                 image.set(P.x, P.y, color);
//             }
//         }
//     }
// }

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head/african_head.obj");
    }

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());
    Vec3f light_dir(0,0,-1);//光照负方向

    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec3f pts[3];
        for (int i=0; i<3; i++) pts[i] = world2screen(model->vert(face[i]));
        Vec3f n=cross(model->vert(face[2])-model->vert(face[0]),model->vert(face[1])-model->vert(face[0]));
        n.normalize();//归一化,防止影响光照强度
        float intensity = n*light_dir;//光照强度
        if (intensity>0) //小于0进行背面剔除,即不显示背面
        {
            std::cout<<intensity<<std::endl;
            triangle(pts, zbuffer,image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}

