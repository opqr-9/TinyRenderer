# 本项目为一个简单的光栅化渲染器

**简介**：本项目使用C++从头编写，未使用opengl和其他库，只从外部使用了模型读取及tga图片写入的函数

**参考**：[ssloy/tinyrenderer: A brief computer graphics / rendering course](https://github.com/ssloy/tinyrenderer)

​	    [从零构建光栅器，tinyrenderer笔记（上）](https://zhuanlan.zhihu.com/p/399056546)，[从零构建光栅器，tinyrenderer笔记（下）](https://zhuanlan.zhihu.com/p/400791821)

## Part 1 Line（画出模型线框）：

<img src="/README_img/1.jpg" style="zoom:25%;" />

<center>图1-1 模型线框</center>

## Part 2 Triangle and Back face culling（模型着色）

<img src="README_img/2.jpg" style="zoom:25%;" />

<center>图2-1 模型着色</center>

## Part 3 Zbuffer（深度检测）	

<img src="README_img/3.jpg" style="zoom:25%;" />

<center>图3-1 深度检测后</center>

## Part 4 Perspective projection （透视矩阵,纹理映射）

<img src="README_img/4-1.jpg" style="zoom:25%;" />

<center>图4-1 ZBuffer</center>

<img src="README_img/4-2.jpg" style="zoom:25%;" />

<center>图4-2 纹理映射</center>

## Part 5 Moving the camera （移动相机）

<img src="README_img/5-1.jpg" style="zoom:25%;" />

<center>图5-1 ZBuffer</center>

<img src="README_img/5-2.jpg" style="zoom:25%;" />

<center>图5-2 摄像机移动后</center>

## Final 

<img src="README_img/final.jpg" style="zoom:25%;" />