# GIFת�빤�ߣ�Video2gif

<br/>

****
## ���
****

Video2gif��һ��֧�ֶ��̴߳����GIFת�빤�ߣ�����FFmpeg��Դ����Ƶ��ܣ������Խ����ַ�װ�������ʽ����Ƶ�ļ�����ת��Ϊ������GIF��ͼ��ͬʱ֧�����š����١�ת���������ڵȡ�

�������ȡ�Ѿ�����ÿ���ֱ��ʹ�õĳ�������[Release](https://github.com/clostou/video2gif/releases/tag/Release)���أ����������滷�����úͰ�װ���ֵ����ݣ�ֱ���Ķ�[ʹ��˵��](https://github.com/clostou/video2gif/tree/Release#%E5%BC%80%E5%A7%8B%E4%BD%BF%E7%94%A8)���֡�

<br/>

****
## �������ã�Windowsƽ̨��
****

**1.��װMSYS2**

��װ������ֱ����MSYS2�������أ�[https://www.msys2.org](https://www.msys2.org/)

**2.��װMinGW��������������**

��`MSYS2 MINGW64`�������������

    pacman -S mingw-w64-x86_64-toolchain make git

���������룺

    git -v
    make -v
    gcc --version

�������ʾ���汾��Ϣ��˵����װ�ɹ���

<br/>

****
## ����װ
****

�������ز���ѹ��Ŀ����ֱ��ʹ��`git`�����¡��

    git clone https://github.com/clostou/video2gif.git

Ȼ�������`MSYS2 MINGW64`�����룺

    cd video2gif
    make

�������ӳɹ��󣬾�����`video2gif/bin`Ŀ¼�¿������ɵĳ���`video2gif.exe`��

<br/>

****
## ��ʼʹ��
****

ֱ�ӽ���ҪתΪGIF����Ƶ�ļ��ϵ������ϣ����ɿ�ʼת�롣

�ڳ����״����к󣬻���ͬĿ¼������һ�������ļ�`video2gif.ini`��������Ҫ�޸Ĳ�������´�ת��ͻᰴ��������ִ�С�

����������ϸ˵�����£�

>* **Image Scale** --- ͼ��(�ֱ���)���š�����ֵΪx0.5ʱ�����GIFͼ��Ŀ�߳ߴ����ԭ��Ƶ��һ�롣
>* **Play Speed** --- ��Ƶ���١�����ֵΪx2.0ʱ�����GIF�Ĳ����ٶ���ԭ��Ƶ��2����
>* **Frame Rate** --- ��Ƶ֡�ʡ���ֵԽ�󣬻���Խ���ᡢ�������ļ�Խ�󣬵����ᳬ��ԭ��Ƶ��֡�ʡ�
>* **Color Depth** --- ɫ����ȡ����洢ͼ��������ɫ�����ݿ�ȣ����Ϊ8λ��
>* **Thread Count** --- ת���߳�����ע��ֵΪ1ʱ���Զ�������ɫ�帴�ã������������ɸ�С��GIF�ļ��������ܻ�ʹ�������覴á�

<br/>

****
## ����
****

�����Ŀ���Ҷ�FFmpeg APIʹ�õĵڶ���ʵս����һ����֮ǰ����һ��������ȡ�ؼ�֡��С����ͬʱ���߱��˶�`C/C++`Ҳ������ѧϰ�׶Σ����в���֮���������ߴͽ̣���ȻҲ��ӭˮƽ������տ�ʼ�Ӵ����������໥����ѧϰ��

��ϵ���䣺*guoqing2234@gmail.com*

<br/>

****
## ����FFmpeg�ı���
****

һ���������ֱ�Ӵ�FFmpeg��������Դ�룬��ʹ��ǰ���õĻ������б��롣����������Ƽ���ͷ��ʼ��ʹ����Ŀ [m-ab-s/media-autobuild_suite](https://github.com/m-ab-s/media-autobuild_suite)���ýű�����������Mingw-w64���뻷���������Զ���װ��������Ƶ�����⣬���Ḻ����

������FFmpeg�������Ĵ�����������ƵתGIF�ж��ò��ϣ�����Ϊ�˽�һ����С����Ĵ�С������������˶Բ��ֳ��ø�ʽ��֧�֡������configure���£�

    ./configure --prefix=/local64/video2gif --pkg-config=pkgconf --cc='ccache gcc' --cxx='ccache g++' --ld='ccache g++' --extra-cxxflags=-fpermissive --extra-cflags=-Wno-int-conversion --arch=x86_64 --enable-gpl --enable-nonfree --disable-programs --disable-autodetect --disable-debug --disable-everything --enable-libx264 --enable-libx265 --enable-libaom --enable-libvpx --enable-demuxer=avi --enable-demuxer=flv --enable-demuxer=matroska --enable-demuxer=mov --enable-demuxer=mpegps --enable-demuxer=mpegts --enable-decoder=mpeg4 --enable-decoder=mpegvideo --enable-decoder=h264 --enable-decoder=hevc --enable-decoder=av1 --enable-decoder=vp8 --enable-decoder=vp9 --enable-decoder=rawvideo --enable-encoder=gif --enable-muxer=gif --enable-filter=scale --enable-filter=split --enable-filter=palettegen --enable-filter=paletteuse --enable-filter=movie --enable-filter=overlay --enable-protocols --disable-network --disable-doc

���ý�����ʼ����FFmpeg����Ҫ���ٱ���ʱ�䣬��һ����Ҫ������`-j`ѡ��ָ�������̣߳�

    make -j8

���������װĿ¼`/local64/video2gif`��

    make install

Ȼ�������`/local64/video2gif/lib`Ŀ¼�¿���С������FFmpeg��̬���ļ��ˡ�

����������֧�ֵĽ��װ������������

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


