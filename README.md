# GIF转码工具：Video2gif

<br/>

****
## 简介
****

Video2gif是一个支持多线程处理的GIF转码工具，借助FFmpeg开源音视频框架，它可以将各种封装及编码格式的视频文件快速转换为高质量GIF动图，同时支持缩放、倍速、转码质量调节等。

若您想获取已经编译好可以直接使用的程序，请点击[Release](https://github.com/clostou/video2gif/releases/tag/Release)下载，并跳过下面环境配置和安装部分的内容，直接阅读[使用说明](https://github.com/clostou/video2gif/tree/Release#%E5%BC%80%E5%A7%8B%E4%BD%BF%E7%94%A8)部分。

<br/>

****
## 环境配置（Windows平台）
****

**1.安装MSYS2**

安装包可以直接在MSYS2官网下载：[https://www.msys2.org](https://www.msys2.org/)

**2.安装MinGW及其他开发工具**

打开`MSYS2 MINGW64`，运行下列命令：

    pacman -S mingw-w64-x86_64-toolchain make git

再依次输入：

    git -v
    make -v
    gcc --version

如果能显示出版本信息，说明安装成功。

<br/>

****
## 程序安装
****

首先下载并解压项目，或直接使用`git`命令克隆：

    git clone https://github.com/clostou/video2gif.git

然后继续在`MSYS2 MINGW64`中输入：

    cd video2gif
    make

编译链接成功后，就能在`video2gif/bin`目录下看到生成的程序`video2gif.exe`。

<br/>

****
## 开始使用
****

直接将需要转为GIF的视频文件拖到程序上，即可开始转码。

在程序首次运行后，会在同目录下生成一个配置文件`video2gif.ini`。根据需要修改并保存后，下次转码就会按照新配置执行。

各参数的详细说明见下：

>* **Image Scale** --- 图像(分辨率)缩放。比如值为x0.5时，输出GIF图像的宽高尺寸均是原视频的一半。
>* **Play Speed** --- 视频倍速。比如值为x2.0时，输出GIF的播放速度是原视频的2倍。
>* **Frame Rate** --- 视频帧率。其值越大，画面越连贯、产生的文件越大，但不会超过原视频的帧率。
>* **Color Depth** --- 色彩深度。即存储图像像素颜色的数据宽度，最大为8位。
>* **Thread Count** --- 转码线程数。注意值为1时会自动开启调色板复用，这有利于生成更小的GIF文件，但可能会使画面产生瑕疵。

<br/>

****
## 其他
****

这个项目是我对FFmpeg API使用的第二次实战，第一次是之前做的一个可以提取关键帧的小程序。同时作者本人对`C/C++`也还处于学习阶段，若有不足之处还望不吝赐教，当然也欢迎水平相近、刚开始接触的朋友来相互交流学习。

联系邮箱：*guoqing2234@gmail.com*

<br/>

****
## 附：FFmpeg的编译
****

一般的做法是直接从FFmpeg官网下载源码，并使用前面搭建好的环境进行编译。不过这里更推荐从头开始就使用项目 [m-ab-s/media-autobuild_suite](https://github.com/m-ab-s/media-autobuild_suite)，该脚本工具内置了Mingw-w64编译环境，且能自动安装各种音视频依赖库，减轻负担。

完整的FFmpeg所包含的大多数组件在视频转GIF中都用不上，另外为了进一步减小程序的大小，这里仅启用了对部分常用格式的支持。具体的configure如下：

    ./configure --prefix=/local64/video2gif --pkg-config=pkgconf --cc='ccache gcc' --cxx='ccache g++' --ld='ccache g++' --extra-cxxflags=-fpermissive --extra-cflags=-Wno-int-conversion --arch=x86_64 --enable-gpl --enable-nonfree --disable-programs --disable-autodetect --disable-debug --disable-everything --enable-libx264 --enable-libx265 --enable-libaom --enable-libvpx --enable-demuxer=avi --enable-demuxer=flv --enable-demuxer=matroska --enable-demuxer=mov --enable-demuxer=mpegps --enable-demuxer=mpegts --enable-decoder=mpeg4 --enable-decoder=mpegvideo --enable-decoder=h264 --enable-decoder=hevc --enable-decoder=av1 --enable-decoder=vp8 --enable-decoder=vp9 --enable-decoder=rawvideo --enable-encoder=gif --enable-muxer=gif --enable-filter=scale --enable-filter=split --enable-filter=palettegen --enable-filter=paletteuse --enable-filter=movie --enable-filter=overlay --enable-protocols --disable-network --disable-doc

配置结束后开始编译FFmpeg。想要减少编译时间，就一定不要忘记用`-j`选项指定复数线程：

    make -j8

最后复制至安装目录`/local64/video2gif`：

    make install

然后就能在`/local64/video2gif/lib`目录下看到小体量的FFmpeg静态库文件了。

下面是启用支持的解封装器及解码器：

- demuxers:
    - avi
    - flv
    - matroska (mkv)
    - mpegps
    - mpegts
    - mov (mp4)

- decoders:
    - av1
    - h264
    - hevc
    - mpeg4
    - mpegvideo
    - rawvideo
    - vp8
    - vp9

<br/>


