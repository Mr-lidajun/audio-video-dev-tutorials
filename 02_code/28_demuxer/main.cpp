#include "mainwindow.h"

#include <QApplication>
#include <iostream>
#include <QtDebug>

extern "C" {
    // 设备
    #include <libavdevice/avdevice.h>
}

void log() {
    // C语言
    printf("printf----");

    // C++
    std::cout << "std:cout-----";

    // FFmpeg
    av_log_set_level(AV_LOG_DEBUG);

    av_log(nullptr, AV_LOG_ERROR, "AV_LOG_ERROR----");
    av_log(nullptr, AV_LOG_WARNING, "AV_LOG_WARNING----");
    av_log(nullptr, AV_LOG_INFO, "AV_LOG_INFO----");

    // 日志级别大小，右边的更大，值越小，越重要
    // TRACE < DEBUG < INFO < WARNING < ERROR < FATAL < QUIET

    // 刷新标准输出流
    fflush(stdout);
    fflush(stderr);

    // Qt
    qDebug() << "qDebug-----";
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

/*
1.创建解封装上下文（打开输入文件，读取文件头）
avformat_open_input
2.检索流信息
avformat_find_stream_info
3.导出流信息到控制台
av_dump_format
4.初始化音频信息
1> 初始化解码器
a) 根据AVMEDIA_TYPE_AUDIO找到音频流，返回音频流的索引
av_find_best_stream
b) 检查音频流是不是空的
c) 为音频流找到合适的解码器
avcodec_find_decoder
d) 创建解码上下文
avcodec_alloc_context3
e) 从音频流中拷贝参数到解码上下文中
avcodec_parameters_to_context
f) 打开解码器
avcodec_open2
2> 打开音频的输出文件
3> 保存音频信息
a) 采样率
b) 采样格式
c) 声道布局
5.初始化视频信息
1> 初始化解码器
a) 根据AVMEDIA_TYPE_VIDEO找到视频流，返回视频流的索引
av_find_best_stream
b) 检查视频流是不是空的
c) 为视频流找到合适的解码器
avcodec_find_decoder
d) 创建解码上下文
avcodec_alloc_context3
e) 从视频流中拷贝参数到解码上下文中
avcodec_parameters_to_context
f) 打开解码器
avcodec_open2
2> 打开视频的输出文件
3> 保存视频信息
a) 宽度、高度
b) 像素格式
c) 帧率
6.初始化AVFrame、AVPacket
7.从输入文件中读取数据，进行解码
1> av_read_frame
2> ...
8.将解码后的数据写到文件中
9.释放资源
*/


/*
640*480，yuv420p
---- 640个Y -----
YY............YY |
YY............YY |
YY............YY |
YY............YY
................ 480行
YY............YY
YY............YY |
YY............YY |
YY............YY |
YY............YY |
---- 320个U -----
UU............UU |
UU............UU |
UU............UU |
UU............UU
................ 240行
UU............UU
UU............UU |
UU............UU |
UU............UU |
UU............UU |
---- 320个V -----
VV............VV |
VV............VV |
VV............VV |
VV............VV
................ 240行
VV............VV
VV............VV |
VV............VV |
VV............VV |
VV............VV |
600*600，rgb24
-------  600个RGB ------
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB
RGB RGB .... RGB RGB 600行
RGB RGB .... RGB RGB
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
RGB RGB .... RGB RGB  |
6 * 4，yuv420p
YYYYYY
YYYYYY
YYYYYY
YYYYYY
UUU
UUU
VVV
VVV
*/
