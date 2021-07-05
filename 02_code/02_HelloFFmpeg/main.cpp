#include "mainwindow.h"

#include <QApplication>

// 导入头文件【也可以不导入，因为<QApplication>中已经包含了<QByteArray>】
// #include <QByteArray>

// 为了使用qDebug函数
#include <QDebug>

// FFmpeg是C语言库
// 有了extern "C"，才能在C++中导入C语言函数
extern "C" {
#include <libavcodec/avcodec.h>
}

int main(int argc, char *argv[])
{
    // 通过qputenv函数设置QT_SCALE_FACTOR为1
    qputenv("QT_SCALE_FACTOR", QByteArray("1"));

    // 打印版本信息
    qDebug() << av_version_info();

    // 创建了一个QApplication对象，在栈中
    // 调用QApplication的构造函数时，传递了2个参数
    // 一个Qt程序中永远只有一个QApplication对象
    QApplication a(argc, argv);

    // 创建主窗口MainWindow对象，在栈中
    MainWindow w;
    // 显示窗口
    w.show();

    // 运行Qt程序
    return a.exec();
}
