#include <vector>
#include <cmath>
#include<iostream>
#include<string.h>
#include "tgaimage.h"
#include"model.h"
#include "geometry.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
const int width  = 1000;
const int height = 1000;

void Swap(int &a,int &b)
{
    int c=a;
    a=b;
    b=c;
}

void line(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)
{
    bool flag=false;
    if(abs((EndY-StartY))>abs((EndX-StartX)))
    {
        Swap(StartX,StartY);
        Swap(EndX,EndY);
        flag=true;
    }
    if(EndX<StartX)
    {
        Swap(EndX,StartX);
        Swap(EndY,StartY);
    }
    int dy=EndY-StartY;
    int dx=EndX-StartX;
    int dslope=abs(dy)*2;//通过乘2去消除上面的四舍五入
    int slope=0;
    for(int x=StartX,y=StartY;x<=EndX;x++)
    {
        if(flag)
        {
            image.set(y,x,color);
        }
        else
        {
            image.set(x,y,color);
        }
        slope+=dslope;
        if(slope>dx)
        {
            y+=(EndY>StartY?1:-1);
            slope-=2*dx;
        }
    }
}

//第一次方法,将三角形分为上下两部分,在同一个y值上取得左右范围并划线,对三角形堆叠等情况不好处理
// void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
//     if (t0.y==t1.y && t0.y==t2.y) return;//判断三点是否在同一直线上
//     //排序t0,t1,t2
//     if (t0.y>t1.y) std::swap(t0, t1);
//     if (t0.y>t2.y) std::swap(t0, t2);
//     if (t1.y>t2.y) std::swap(t1, t2);
//     int totalHigh=t2.y-t0.y;
//     int belowHigh=t1.y-t0.y;
//     int upHigh=t2.y-t1.y;
//     int y=t0.y;
//     // for(;y<t1.y;y++)//下半部分渲染
//     // {
//     //     double tmp1=(y-t0.y)/(double)totalHigh;
//     //     double tmp2=(y-t0.y)/(double)belowHigh;
//     //     Vec2i left=t0+(t2-t0)*tmp1;
//     //     Vec2i right=t0+(t1-t0)*tmp2;
//     //     //line(left.x,y,right.x,y,image,color);
//     //     //以下功能等效于上面的line()
//     //     if(left.x>right.x)
//     //     {
//     //         std::swap(left,right);
//     //     }
//     //     for(int i=left.x;i<right.x;i++)
//     //     {
//     //         image.set(i,y,color);
//     //     }
//     // }
//     // for(;y<=t2.y;y++)//上半部分渲染
//     // {
//     //     double tmp1=(y-t0.y)/(double)totalHigh;
//     //     double tmp2=(y-t1.y)/(double)upHigh;
//     //     Vec2i left=t0+(t2-t0)*tmp1;
//     //     Vec2i right=t1+(t2-t1)*tmp2;
//     //     //line(left.x,y,right.x,y,image,color);
//     //     //以下功能等效于上面的line()
//     //     if(left.x>right.x)
//     //     {
//     //         std::swap(left,right);
//     //     }
//     //     for(int i=left.x;i<right.x;i++)
//     //     {
//     //         image.set(i,y,color);
//     //     }
//     // }
//     for(;y<t2.y;y++)//通用
//     {
//         bool flag=(y<t1.y) || (t1.y==t2.y);//后半部分防止倒的平行x轴底边使得upHigh为0,导致除法失效
//         double tmp1=(y-t0.y)/(double)totalHigh;
//         double tmp2=flag?(y-t0.y)/(double)belowHigh:(y-t1.y)/(double)upHigh;
//         Vec2i left=t0+(t2-t0)*tmp1;
//         Vec2i right=flag?t0+(t1-t0)*tmp2:t1+(t2-t1)*tmp2;
//         //line(left.x,y,right.x,y,image,color);
//         //以下功能等效于上面的line()
//         if(left.x>right.x)
//         {
//             std::swap(left,right);
//         }
//         for(int i=left.x;i<right.x;i++)
//         {
//             image.set(i,y,color);
//         }
//     }
// }

//第二种方法,使用包围盒包围三角形，对点逐个判断是否在三角形内
//计算重心坐标,计算原理在备注中
Vec3f barycentric(Vec2i* pts, Vec2i p) 
{
    Vec2i AB=pts[1]-pts[0];
    Vec2i AC=pts[2]-pts[0];
    Vec2i PA=pts[0]-p;
    Vec3f tmpx=Vec3f(AB.x,AC.x,PA.x);
    Vec3f tmpy=Vec3f(AB.y,AC.y,PA.y);
    Vec3f uvLocation=tmpx^tmpy;//叉乘
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

void triangle(Vec2i* pts,TGAImage &image, TGAColor color)
{
    Vec2i boxmax(pts[0].x,pts[0].y);
    Vec2i boxmin(pts[0].x,pts[0].y);
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
    Vec2i tmp=boxmin;
    for(;tmp.x<boxmax.x;(tmp.x)++)
    {
        for(tmp.y=boxmin.y;tmp.y<boxmax.y;(tmp.y)++)
        {
            Vec3f uv=barycentric(pts,tmp);
            if(uv.x>=0 && uv.y>=0 && uv.z>=0)
            {
                image.set(tmp.x,tmp.y,color);
            }
        }
    }
}

//第一种方法测试
// int main(int argc, char** argv) {
//     TGAImage image(width, height, TGAImage::RGB);
//     Vec2i t0[3] = {Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80)};
//     Vec2i t1[3] = {Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180)};
//     Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};
//     triangle(t0[0], t0[1], t0[2], image, red);
//     triangle(t1[0], t1[1], t1[2], image, white);
//     triangle(t2[0], t2[1], t2[2], image, green);
//     image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
//     image.write_tga_file("output.tga");
//     return 0;
// }

//第二种方法测试
// int main(int argc, char** argv) { 
//     TGAImage frame(200, 200, TGAImage::RGB); 
//     Vec2i pts[3] = {Vec2i(10,10), Vec2i(100, 30), Vec2i(190, 160)}; 
//     triangle2(pts, frame, TGAColor(255, 0, 0, 255)); 
//     frame.flip_vertically(); // to place the origin in the bottom left corner of the image 
//     frame.write_tga_file("framebuffer.tga");
//     return 0; 
// }

//模型测试
int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head.obj");
    }

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);//光照负方向

    //随机颜色测试
    // for (int i=0; i<model->nfaces(); i++) { 
    //     std::vector<int> face = model->face(i); 
    //     Vec2i screen_coords[3]; 
    //     for (int j=0; j<3; j++) { 
    //         Vec3f world_coords = model->vert(face[j]); 
    //         screen_coords[j] = Vec2i((world_coords.x+1.)*width/2., (world_coords.y+1.)*height/2.); 
    //     } 
    //     triangle(screen_coords, image, TGAColor(rand()%255, rand()%255, rand()%255, 255)); 
    // }

    //添加光照,未处理zbuffer
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i);
        Vec2i screen_coords[3];
        Vec3f world_coords[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(face[j]);
            screen_coords[j] = Vec2i((v.x+1.)*width/2., (v.y+1.)*height/2.);//模型坐标为(-1,-1)到(1,1)，映射到世界坐标为(0,0)到(width,height),不考虑z轴
            world_coords[j]  = v;
        }
        Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);//法向量
        n.normalize();//归一化,防止影响光照强度
        float intensity = n*light_dir;//光照强度
        if (intensity>0) //小于0进行背面剔除,即不显示背面
        {
            triangle(screen_coords, image, TGAColor(intensity*255, intensity*255, intensity*255, 255));
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}