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
