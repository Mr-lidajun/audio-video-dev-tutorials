#include "playthread.h"
#include <QThread>
#include <QtDebug>

#include <SDL2/SDL.h>
#include <QFile>

/**定义条件宏*/
#define END(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        goto end; \
    }

#define FILENAME "D:/Dev/C_CPP/test/in.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 512
#define IMG_H 512

PlayThread::PlayThread(void *winId, QObject *parent)
    : QThread(parent), _winId(winId)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &PlayThread::finished,
            this, &PlayThread::deleteLater);
}

// 析构函数
PlayThread::~PlayThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

void PlayThread::run() {
    // 窗口
    SDL_Window *window = nullptr;

    // 渲染上下文
    SDL_Renderer *renderer = nullptr;

    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *texture = nullptr;

    // 文件
    QFile file(FILENAME);

    // 初始化Video子系统
    END(SDL_Init(SDL_INIT_VIDEO), SDL_Init);


    // 创建一个窗口
    window = SDL_CreateWindowFrom(_winId);
    END(!window, SDL_CreateWindow);

    // 创建硬件加速的渲染上下文
    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    // 如果创建硬件加速的渲染上下文失败，则表示不支持硬件加速，就传0
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
        END(!renderer, SDL_CreateRenderer);
    }

    // 创建纹理
    texture = SDL_CreateTexture(renderer,
                                PIXEL_FORMAT,
                                SDL_TEXTUREACCESS_STREAMING,
                                IMG_W, IMG_H);
    END(!texture, SDL_CreateTexture);

    // 打开文件
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
        goto end;
    }

    // 将YUV的像素数据填充到纹理texture
    END(SDL_UpdateTexture(texture, nullptr, file.readAll().data(),IMG_W),
        SDL_UpdateTexture);

    // 设置绘制颜色（画笔颜色）
    END(SDL_SetRenderDrawColor(renderer,
                               0, 0, 0, SDL_ALPHA_OPAQUE),
        SDL_SetRenderDrawColor);

    // 设置绘制颜色（画笔颜色）清除渲染目标
    END(SDL_RenderClear(renderer), SDL_RenderClear);

    // 拷贝纹理到渲染目标上（默认是window）
    END(SDL_RenderCopy(renderer, texture, nullptr, nullptr), SDL_RenderCopy);

    // 更新所有的渲染操作到屏幕上
    SDL_RenderPresent(renderer);

    // 等待退出事件
    while (true) {
        if (isInterruptionRequested()) {
            goto end;
        }
    }

end:
    file.close();
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    // 清除所有的子系统
    SDL_Quit();

    qDebug() << "线程执行完毕";
}


