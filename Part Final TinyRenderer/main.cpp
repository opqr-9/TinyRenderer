/*实现一个自己的光栅化渲染器
//借用的类: 矩阵类  向量类  tga图片编辑类   模型读取类
*/
#include "LendClass.h"

const int width  = 1000;//图片宽度
const int height = 1000;//图片高度
const int depth  = 255;//深度
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
Model *model = NULL;
Matrix ViewPort;
Matrix Projection;
Matrix ModelView;
Vec3f light_dir=Vec3f(-1,0,-1).normalize();
Vec3f anti_light_dir=light_dir*-1;//光线负方向
Vec3f eye(5,1,10);//相机位置
Vec3f center(0,0,0);//视角中心
Vec3f up(0,1,0);
float zBuffer[width][height];

//向量交换
void Swap(Vec2i &v1,Vec2i &v2)
{
    Vec2i v(v2);
    v2.y=v1.y;v2.x=v1.x;
    v1.y=v.y;v1.x=v.x;
}
//数值交换
void Swap(float &num1,float &num2)
{
    float num=num2;
    num2=num1;
    num1=num;
}
void Swap(int &num1,int &num2)
{
    int num=num2;
    num2=num1;
    num1=num;
}
//画线
//不能画平行于y轴和x轴的线
void Line(Vec2i v1,Vec2i v2,TGAColor color,TGAImage &image)
{
    bool flag=false;

    if(abs(v2.y-v1.y)>abs(v2.x-v1.x))//判断斜率绝对值是否大于一,如果是则交换x坐标为y坐标,y坐标为x坐标,防止因斜率过大导致线段渲染点数过少
    {
        Swap(v1.x,v1.y);
        Swap(v2.x,v2.y);
        flag=true;
    }

    if(v1.x>v2.x)//判断v2的x坐标是否大于v1的x坐标，以便进行x增量渲染
    {
        Swap(v1,v2);
    }

    int dy=abs((v2.y-v1.y)*2);
    int dx=(v2.x-v1.x)*2;
    /*"/*"所示出的地方为优化原理
     float derror=abs(dy/dx);
    */
    int error=0;
    for(int i=v1.x,y=v1.y;i<=v2.x;i++)
    {
        if(flag)
        {
            image.set(y,i,color);
        }
        else
        {
            image.set(i,y,color);
        }
        error+=dy;
        if(error>dx)
        {
            y+=(v2.y>v1.y?1:-1);
            error-=dx;
        }
        /*error+=derror;
        // if(error>1)
        // {
        //     y+=(v2.y>v1.y?1:-1);
        //     error-=1;
        // }
        */
    }
}
//上下分开通过画线渲染画三角形
//缺点:以画线为单位动作,不好进行并行处理
void LineTriangle(Vec2i *v,TGAColor color,TGAImage &image)
{
    if(v[0].y==v[1].y&&v[1].y==v[2].y) return;//防止是直线
    if(v[0].y>v[1].y){Swap(v[0],v[1]);}//将三个点根据y值进行排序
    if(v[1].y>v[2].y){Swap(v[1],v[2]);}
    if(v[0].y>v[2].y){Swap(v[0],v[2]);}

    int totalHeight=v[2].y-v[0].y;
    int belowHeight=v[1].y-v[0].y;
    int upHeight=v[2].y-v[1].y;

    int y=v[0].y;
    //整合渲染
    for(int bottom=0;y<v[2].y;y++)
    {
        if(y==v[1].y)
        {
            bottom=1;
        }
        float totalPercent=(y-v[0].y)/(float)totalHeight;
        float partPercent=(y-v[bottom].y)/((bottom==1)?(float)upHeight:(float)belowHeight);
        int leftX=totalPercent*(v[2].x-v[0].x)+v[0].x;
        int rightX=partPercent*(v[bottom+1].x-v[bottom].x)+v[bottom].x;
        if(rightX<leftX)
        {
            Swap(leftX,rightX);
        }
        //画直线
        for(int x=leftX;x<=rightX;x++)
        {
            image.set(x,y,color);
        }
    }
    return ;
    //以下为常规思路渲染
    //三角形下半部分渲染
    for(;y<=v[1].y;y++)
    {
        float totalPercent=(y-v[0].y)/(float)totalHeight;
        float belowPercent=(y-v[0].y)/(float)belowHeight;
        int leftX=totalPercent*(v[2].x-v[0].x)+v[0].x;
        int rightX=belowPercent*(v[1].x-v[0].x)+v[0].x;
        if(rightX<leftX)
        {
            Swap(leftX,rightX);
        }
        //画直线
        for(int x=leftX;x<=rightX;x++)
        {
            image.set(x,y,color);
        }
    }
    //三角形上半部分渲染
    for(;y<v[2].y;y++)
    {
        float totalPercent=(y-v[0].y)/(float)totalHeight;
        float upPercent=(y-v[1].y)/(float)upHeight;
        int leftX=totalPercent*(v[2].x-v[0].x)+v[0].x;
        int rightX=upPercent*(v[2].x-v[1].x)+v[1].x;
        if(rightX<leftX)
        {
            Swap(leftX,rightX);
        }
        //画直线
        for(int x=leftX;x<=rightX;x++)
        {
            image.set(x,y,color);
        }
    }
}

//求点的重心坐标,原理在备注中
Vec3f Barycentric(Vec3f *v,Vec3f p)
{
    Vec3f AB=v[1]-v[0];
    Vec3f AC=v[2]-v[0];
    Vec3f PA=v[0]-p;
    Vec3f x(AB.x,AC.x,PA.x),y(AB.y,AC.y,PA.y);
    Vec3f res=x^y;
    return Vec3f(1-(res.x+res.y)/res.z,res.x/res.z,res.y/res.z);
}

//使用包围盒包围三角形,逐点求其重心坐标以便确认是否在三角形内
void BarycentricTriangle(Vec3f *v,Vec2i *uv,float *intensity, TGAImage &image)
{
    Vec2i min(v[0].x,v[0].y);
    Vec2i max(v[0].x,v[0].y);

    //判断左下点和右上点
    for(int i=1;i<3;i++)
    {
        if(v[i].x<min.x)
        {
            min.x=v[i].x;
        }
        else if(v[i].x>max.x)
        {
            max.x=v[i].x;
        }
        if(v[i].y<min.y)
        {
            min.y=v[i].y;
        }
        else if(v[i].y>max.y)
        {
            max.y=v[i].y;
        }
    }
    //避免超出图片范围,即width和height
    max.x=max.x<width-1?max.x:width-1;
    max.y=max.y<height-1?max.y:height-1;
    min.x=min.x>0?min.x:0;
    min.y=min.y>0?min.y:0;

    Vec3f target;
    for(target.x=min.x;target.x<=max.x;target.x++)
    {
        for(target.y=min.y;target.y<=max.y;target.y++)
        {
            Vec3f arealCoord=Barycentric(v,target);

            if(arealCoord.x>=0&&arealCoord.y>=0&&arealCoord.z>=0)//等于0是用来渲染边界
            {
                target.z=0;
                Vec2i uvTarget;
                float intensityTarget=0;
                for(int i=0;i<3;i++)
                {
                    uvTarget.x+=uv[i].x*arealCoord[i];//插值uv坐标
                    uvTarget.y+=uv[i].y*arealCoord[i];
                    target.z+=v[i].z*arealCoord[i];//插值z深度
                    intensityTarget+=intensity[i]*arealCoord[i];//插值光照强度
                    
                }
                if(zBuffer[(int)target.x][(int)target.y]<target.z)
                {
                    zBuffer[(int)target.x][(int)target.y]=target.z;
                    TGAColor color=model->diffuse(uvTarget);
                    image.set(target.x,target.y,TGAColor(color.bgra[2], color.bgra[1], color.bgra[0])*intensityTarget);
                }
            }
        }
    }
}

//坐标转齐次坐标矩阵
Matrix VectorToMatrix4(Vec3f v)
{
    Matrix m(4,1);
    m[0][0]=v.x;
    m[1][0]=v.y;
    m[2][0]=v.z;
    m[3][0]=1;
    return m;
}

//齐次坐标矩阵转坐标
Vec3f MatrixToVector4(Matrix m)
{
    Vec3f v;
    v.x=m[0][0]/m[3][0];
    v.x=m[1][0]/m[3][0];
    v.x=m[2][0]/m[3][0];
    return v;
}

void WorldToScreen(Vec3f *v,Vec3f *output) 
{
    for(int i=0;i<3;i++)
    {
        Matrix m=VectorToMatrix4(v[i]);
        output[i]=ViewPort*Projection*ModelView*m;
    }
}

//移动相机,原理是利用相机的移动获得变化的逆矩阵从而移动物体
//TODO:up向量一般为(0,1,0)(原本世界的y轴),除非相机要自旋(赛车游戏桶滚),up向量与相机的z轴和相机的y轴在同一平面
//逆矩阵获取:利用摄像机一直朝向-z轴(一般只有z轴或-z轴)获取相机移动后的z轴,再将相机的z轴叉乘up向量得到相机的x轴,相机的x轴叉乘z轴获得相机的y轴,从而获取相机的变化矩阵,再获取逆矩阵
Matrix Lookat(Vec3f up)
{
    Matrix m=Matrix::identity(4);
    Vec3f x,y,z;
    Vec3f cameracoord[3];
    z=(eye-center).normalize();
    x=(up^z).normalize();
    y=(z^x).normalize();
    //因为相机的x,y,z轴构成正交矩阵,而正交矩阵的逆为正交矩阵的转置,此处已经转置过了
    for(int i=0;i<3;i++)
    {
        m[0][i] = x[i];
        m[1][i] = y[i];
        m[2][i] = z[i];
        m[i][3] = -center[i];
    }
    return m;
}

//透视矩阵,视角矩阵,相机移动矩阵初始化
void MatrixInit()
{
    
    //视角矩阵,将模型坐标为(-1,-1,-1)到(1,1,1)，映射到世界坐标为(0,0,0)到(width,height,depth)
    // Matrix ViewPort={
    //                     width/2 ,0        ,0       ,width/2  ,
    //                     0       ,height/2 ,0       ,height/2 ,  
    //                     0       ,0        ,depth/2 ,depth/2  ,
    //                     0       ,0        ,0       ,1          
    //                  };
    ViewPort=Matrix::identity(4);
    ViewPort[0][0]=width/2;
    ViewPort[1][1]=height/2;
    ViewPort[2][2]=depth/2;

    ViewPort[0][3]=width/2;
    ViewPort[1][3]=height/2;
    ViewPort[2][3]=depth/2;

    //透视矩阵,实现透视
    // Matrix Projection={ 1    ,0    ,0    ,0    ,
    //                     0    ,1    ,0    ,0    ,  
    //                     0    ,0    ,1    ,0    ,
    //                     0    ,0    ,-1/c ,1    };
    Projection=Matrix::identity(4);
    Projection[3][2]=-1.f/eye.z;

    ModelView=Lookat(up);
}

int main(int argc, char** argv)
{
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("obj/african_head/african_head.obj");
    }
    TGAImage image(width,height,TGAImage::RGB);
    
    MatrixInit();

    for (int x=0; x<width; x++)//将zBuffer初始化为极小值
    {
        for(int y=0;y<height;y++)
        {
            zBuffer[x][y] = -std::numeric_limits<float>::max();
        }
    }

    for(int i=0;i<model->nfaces();i++)
    {
        std::vector<int> face=model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec3f v[3];
        for(int j=0;j<3;j++)
        {
            v[j]=model->vert(face[j]);
        }
        WorldToScreen(v,screen_coords);
        float intensity[3];
        if (intensity>0)
        {
            Vec2i uv[3];//获取该点的纹理
            for (int j=0; j<3; j++) {
                uv[j] = model->uv(i,j);
                intensity[j]=model->norm(i,j)*anti_light_dir;
            }
            BarycentricTriangle(screen_coords,uv,intensity,image);
        }
    }

    // Vec2f v1(10,50),v2(50,400);
    // Line(v1,v2,red,image);

    // Vec2i v[3]={Vec2i(10,10),Vec2i(400,200),Vec2i(500,600)};
    // BarycentricTriangle(v,red,image);

    image.flip_vertically();//因为tga图片的左上角为原点
    image.write_tga_file("output.tga");
    return 0;
}