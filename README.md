# 本项目为一个简单的光栅化渲染器

**简介**：本项目使用C++从头编写，未使用opengl和其他库，只从外部使用了模型读取及tga图片写入的函数

**参考**：[ssloy/tinyrenderer: A brief computer graphics / rendering course](https://github.com/ssloy/tinyrenderer)，[从零构建光栅器，tinyrenderer笔记（上）](https://zhuanlan.zhihu.com/p/399056546)，[从零构建光栅器，tinyrenderer笔记（下）](https://zhuanlan.zhihu.com/p/400791821)

## Part 1 Line（画出模型线框）：

<div align="center"><img src="/README_img/1.jpg" width="300" /></div>

<p align="center">图1-1 模型线框</p>

## Part 2 Triangle and Back face culling（模型着色）

<div align="center"><img src="README_img/2.jpg" width="300" /></div>

<p align="center">图2-1 模型着色</p>

## Part 3 Zbuffer（深度检测）	

<div align="center"><img src="README_img/3.jpg" width="300" /></div>

<p align="center">图3-1 深度检测后</p>

## Part 4 Perspective projection （透视矩阵,纹理映射）

<div align="center"><img src="README_img/4-1.jpg" width="300" /></div>

<p align="center">图4-1 ZBuffer</p>

<div align="center"><img src="README_img/4-2.jpg" width="300" /></div>

<p align="center">图4-2 纹理映射</p>

## Part 5 Moving the camera （移动相机）

<div align="center"><img src="README_img/5-1.jpg" width="300" /></div>

<p align="center">图5-1 ZBuffer</p>

<div align="center"><img src="README_img/5-2.jpg" width="300" /></div>

<p align="center">图5-2 摄像机移动后</p>

## Final 

<div align="center"><img src="README_img/final.jpg" width="300" /></div>

<p align="center">图6-1 最后样式</p>
