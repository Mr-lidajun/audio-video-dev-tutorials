#include "mainwindow.h"

#include <QApplication>

// 终止宏定义命令：#define main    SDL_main
#undef main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
