/*此处为借用的类所使用的头文件
//借用的类: 矩阵类  向量类  tga图片编辑类   模型读取类
*/

#ifndef __LENDCLASS_H__
#define __LENDCLASS_H__

#include <cmath>
#include <vector>
#include <iostream>
#include <cstring>
#include <limits>

class Matrix;


//二维向量模板类
template <class t> struct Vec2 {
    t x, y;
    Vec2<t>() : x(t()), y(t()) {}
    Vec2<t>(t _x, t _y) : x(_x), y(_y) {}
    Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(x+V.x, y+V.y); }
    Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(x-V.x, y-V.y); }
    Vec2<t> operator *(float f)          const { return Vec2<t>(x*f, y*f); }//点乘
    t& operator[](const int i) { return i<=0 ? x : y; }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};
//三维向量模板类
template <class t> struct Vec3 {
    t x, y, z;
    Vec3<t>() : x(t()), y(t()), z(t()) { }
    Vec3<t>(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}
    Vec3<t>(Matrix m);
    template <class u> Vec3<t>(const Vec3<u> &v);
    Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }//叉乘
    Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
    Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
    Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }//点乘
    t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }//点乘
    float norm () const { return std::sqrt(x*x+y*y+z*z); }//范数
    Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }//归一化
    t& operator[](const int i) { return i<=0 ? x : (1==i ? y : z); }
    template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};
//别名
typedef Vec2<float> Vec2f;
typedef Vec2<int>   Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int>   Vec3i;
//拷贝构造函数
template <> template <> Vec3<int>::Vec3(const Vec3<float> &v);
template <> template <> Vec3<float>::Vec3(const Vec3<int> &v);
//重载输出操作符
template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
    s << "(" << v.x << ", " << v.y << ")\n";
    return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
    return s;
}


//矩阵类
class Matrix {
    std::vector<std::vector<float> > m;
    int rows, cols;
public:
    Matrix(int r=4, int c=4);
    Matrix(Vec3f v);
    int nrows();
    int ncols();
    static Matrix identity(int dimensions);
    std::vector<float>& operator[](const int i);
    Matrix operator*(const Matrix& a);//矩阵乘法
    Matrix transpose();//矩阵转置
    Matrix inverse();//逆矩阵
    friend std::ostream& operator<<(std::ostream& s, Matrix& m);
};


//指定结构和联合成员的紧缩对齐
#pragma pack(push,1)
struct TGA_Header {
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char  bitsperpixel;
    char  imagedescriptor;
};
//还原
#pragma pack(pop)
//tga图片颜色类
struct TGAColor {
    unsigned char bgra[4];
    unsigned char bytespp;

    TGAColor() : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
    }

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A=255) : bgra(), bytespp(4) {
        bgra[0] = B;
        bgra[1] = G;
        bgra[2] = R;
        bgra[3] = A;
    }

    TGAColor(unsigned char v) : bgra(), bytespp(1) {
        for (int i=0; i<4; i++) bgra[i] = 0;
        bgra[0] = v;
    }


    TGAColor(const unsigned char *p, unsigned char bpp) : bgra(), bytespp(bpp) {
        for (int i=0; i<(int)bpp; i++) {
            bgra[i] = p[i];
        }
        for (int i=bpp; i<4; i++) {
            bgra[i] = 0;
        }
    }

    //与光照强度相乘
    TGAColor operator *(float intensity) const {
        TGAColor res = *this;
        intensity = (intensity>1.f?1.f:(intensity<0.f?0.f:intensity));
        for (int i=0; i<4; i++) res.bgra[i] = bgra[i]*intensity;
        return res;
    }
};
//tga图片编辑类
class TGAImage {
protected:
    unsigned char* data;
    int width;
    int height;
    int bytespp;

    bool   load_rle_data(std::ifstream &in);//加载tga图片数据(可能?)////////////////////////////////////////////////////////////////////////////////////////////////////////////
    bool unload_rle_data(std::ofstream &out);//卸载tga图片数据(可能?)////////////////////////////////////////////////////////////////////////////////////////////////////////////
public:
    enum Format {
        GRAYSCALE=1, RGB=3, RGBA=4
    };

    TGAImage();
    TGAImage(int w, int h, int bpp);
    TGAImage(const TGAImage &img);
    bool read_tga_file(const char *filename);//读取tga图片
    bool write_tga_file(const char *filename, bool rle=true);//写入tga图片
    bool flip_horizontally();//图片水平翻转
    bool flip_vertically();//图片竖直翻转
    bool scale(int w, int h);//缩放图片
    TGAColor get(int x, int y);//获取某点颜色
    bool set(int x, int y, TGAColor &c);//设置某点颜色
    bool set(int x, int y, const TGAColor &c);//设置某点颜色
    ~TGAImage();
    TGAImage & operator =(const TGAImage &img);
    int get_width();//获取图片宽
    int get_height();//获取图片高
    int get_bytespp();//获取?////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned char *buffer();
    void clear();
};


//模型类
class Model {
    private:
        std::vector<Vec3f> verts_;
        std::vector<std::vector<Vec3i> > faces_; // attention, this Vec3i means vertex/uv/normal
        std::vector<Vec3f> norms_;
        std::vector<Vec2f> uv_;
        TGAImage diffusemap_;
        void load_texture(std::string filename, const char *suffix, TGAImage &img);//载入纹理
    public:
        Model(const char *filename);
        ~Model();
        int nverts();//获取顶点数
        int nfaces();//获取面数
        Vec3f norm(int iface, int nvert);//获取归一化后的法向量
        Vec3f vert(int i);//获取第i+1个顶点
        Vec2i uv(int iface, int nvert);//获取uv(可能?)////////////////////////////////////////////////////////////////////////////////////////////////////////////
        TGAColor diffuse(Vec2i uv);//?功能////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::vector<int> face(int idx);//获取第idx+1面的所有顶点的序号
    };

#endif //__LENDCLASS_H__
