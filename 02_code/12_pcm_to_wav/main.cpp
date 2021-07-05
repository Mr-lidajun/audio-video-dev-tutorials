#include "mainwindow.h"

#include <QApplication>
#include <iostream>
#include <QtDebug>
#include "ffmpegs.h"

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
//    WAVHeader header;
//    header.sampleRate = 44100;
//    header.bitsPerSample = 16;
//    header.numChannels = 2;

//    // pcm转wav文件
//    FFmpegs::pcm2wav(header, "D:/Dev/C_CPP/test/in.pcm", "D:/Dev/C_CPP/test/in.wav");

    // 初始化libavdevice并注册所有输入和输出设备
    avdevice_register_all();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
