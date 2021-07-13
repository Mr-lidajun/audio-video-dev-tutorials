#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <SDL2/SDL.h>

// 终止宏定义命令：#define main    SDL_main
#undef main

int main(int argc, char *argv[])
{
    // 初始化Video子系统
    if (SDL_Init(SDL_INIT_VIDEO)) {
        qDebug() << "SDL_Init error" << SDL_GetError();
        return 0;
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    int ret = a.exec();

    // 清除所有的子系统
    SDL_Quit();

    return ret;
}
