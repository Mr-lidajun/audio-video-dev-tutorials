#include "mainwindow.h"

#include <QApplication>
#include "ffmpegs.h"

// 终止宏定义命令：#define main    SDL_main
#undef main

int main(int argc, char *argv[])
{
//    RawVideoFile in = {
//        "D:/Dev/C_CPP/test/out_heike.yuv",
//        624, 368, AV_PIX_FMT_YUV420P
//    };
//    RawVideoFile out = {
//        "D:/Dev/C_CPP/test/out_heike.rgb",
//        512, 512, AV_PIX_FMT_RGB24
//    };
//    FFmpegs::convertRawVideo(in, out);

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
