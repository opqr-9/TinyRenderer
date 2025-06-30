#include"tgaimage.h"
#include"geometry.h"
#include"model.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

class LineRenderer
{
public:
    virtual void Try(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)=0;
};

//第一次尝试,使用i(0.01)倍步长生成线段
//弊端:无论线段长短都会生成1/i(总计100)个点
class First:public LineRenderer
{
    void Try(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)
    {
        for(double i=0;i<1;i+=0.01)
        {
            int x=StartX+(EndX-StartX)*i;
            int y=StartY+(EndY-StartY)*i;
            image.set(x,y,color);
        }
    }
};

//第二次尝试,使用x,以1为单位距离生成线段
//弊端:在斜率极大时导致线段断裂,且只能在EndX大于StartX下适用
class Second:public LineRenderer
{
    void Try(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)
    {
        for(int x=StartX;x<=EndX;x++)
        {
            double precent=(double)(x-StartX)/(double)(EndX-StartX);
            int y=precent*(EndY-StartY)+StartY;
            image.set(x,y,color);
        }
    }
};
//第三次尝试,延续第二次尝试方法,当EndX大于StartX时，交换二者，当斜率极大时，交换StartX,StartY,EndX,EndY
void Swap(int &a,int &b)
{
    int c=a;
    a=b;
    b=c;
}

class Third:public LineRenderer
{
    void Try(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)
    {
        bool flag=false;//判断当前线段斜率是否大于1
        if(abs((EndY-StartY))>abs((EndX-StartX)))//如果斜率大于1,调换x,y的坐标
        {
            Swap(StartX,StartY);
            Swap(EndX,EndY);
            flag=true;
        }
        if(EndX<StartX)//如果终点比起点小,调换端点坐标
        {
            Swap(EndX,StartX);
            Swap(EndY,StartY);
        }
        for(int x=StartX;x<=EndX;x++)
        {
            double precent=(x-StartX)/(double)(EndX-StartX);
            int y=precent*(EndY-StartY)+StartY;
            if(flag)
            {
                image.set(y,x,color);
            }
            else
            {
                image.set(x,y,color);
            }
        }
    }
};

//优化:提取循环中的重复变量,利用斜率的累加代替第三次尝试percent的乘积
class Final:public LineRenderer
{
    void Try(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)
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
        double dslope=abs((EndY-StartY)/(double)(EndX-StartX));//斜率
        double slope=0;
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
            slope+=dslope;//相当于每次随着x坐标加1,y加1*斜率
            if(slope>0.5)//四舍五入
            {
                y+=(EndY>StartY?1:-1);
                slope--;
            }
        }
    }
};

//最后优化:去除浮点数
class FinalPro:public LineRenderer
{
    void Try(int StartX,int StartY,int EndX,int EndY,TGAImage &image,TGAColor color)
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
};

//线段测试
// int main()
// {
//     TGAImage image(100,100,TGAImage::RGB);
//     LineRenderer* lineRenderer=new FinalPro();
//     lineRenderer->Try(13,20,80,40,image,white);
//     lineRenderer->Try(20, 13, 40, 80, image, red);
//     lineRenderer->Try(80, 40, 13, 20, image, red);
//     image.flip_vertically();
//     image.write_tga_file("output.tga");
//     return 0;
// }

int main(int argc, char** argv){
    if (2==argc) {
        model = new Model(argv[1]);  //命令行控制方式构造model
    } else {
        model = new Model("obj/african_head.obj"); //代码方式构造model
    }
    LineRenderer* lineRenderer=new FinalPro();
    TGAImage image(width, height, TGAImage::RGB);
    for (int i=0; i<model->nfaces(); i++) {
        std::vector<int> face = model->face(i); //创建face数组用于保存一个face的三个顶点坐标
        for (int j=0; j<3; j++) {
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);
            //根据顶点v0和v1画线
            //先要进行模型坐标到屏幕坐标的转换。  (-1,-1)对应(0,0)   (1,1)对应(width,height)
            int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;
            //画线
            lineRenderer->Try(x0,y0, x1,y1,image,white);
            // line(x0,y0, x1,y1, image, white);
        }
    }
    image.flip_vertically();
    image.write_tga_file("output.tga");
}