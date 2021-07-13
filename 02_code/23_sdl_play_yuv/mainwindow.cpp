#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <SDL2/SDL.h>
#include <QDebug>
#include <QFile>

/**定义条件宏*/
#define RET(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        return; \
    }

#define FILENAME "D:/Dev/C_CPP/test/out_heike.yuv"
#define PIXEL_FORMAT SDL_PIXELFORMAT_IYUV
#define IMG_W 624
#define IMG_H 368

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置窗口大小
    resize(640, 580);

    // 创建播放器界面
    _widget = new QWidget(this);
    _widget->setGeometry(50, 50, IMG_W, IMG_H);

    // 初始化Video子系统
    RET(SDL_Init(SDL_INIT_VIDEO), SDL_Init);


    // 创建一个窗口
    _window = SDL_CreateWindowFrom((void *)_widget->winId());
    RET(!_window, SDL_CreateWindow);

    // 创建硬件加速的渲染上下文
    _renderer = SDL_CreateRenderer(_window, -1,
                                  SDL_RENDERER_ACCELERATED |
                                  SDL_RENDERER_PRESENTVSYNC);
    // 如果创建硬件加速的渲染上下文失败，则表示不支持硬件加速，就传0
    if (!_renderer) {
        _renderer = SDL_CreateRenderer(_window, -1, 0);
        RET(!_renderer, SDL_CreateRenderer);
    }

    // 创建纹理
    _texture = SDL_CreateTexture(_renderer,
                                PIXEL_FORMAT,
                                SDL_TEXTUREACCESS_STREAMING,
                                IMG_W, IMG_H);
    RET(!_texture, SDL_CreateTexture);

    // 打开文件
    _file.setFileName(FILENAME);
    if (!_file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << FILENAME;
    }
}

MainWindow::~MainWindow()
{
    delete ui;

    _file.close();
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
    // 清除所有的子系统
    SDL_Quit();
}

void showVersion() {
    SDL_version version;
    SDL_VERSION(&version);
    qDebug() << version.major << version.minor << version.patch;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    qDebug() << "closeEvent";
}

void MainWindow::on_playButton_clicked()
{
    // 开启定时器
    _timerId = startTimer(33);
}

// 每隔一段时间就会调用
void MainWindow::timerEvent(QTimerEvent *event) {
    // 存储格式为：I420(yuv420p)，1个像素平均占用12bit（1.5字节）
    // 一帧图片的大小
    int imgSize = IMG_W * IMG_H * 1.5;
    char data[imgSize];

    if (_file.read(data, imgSize) > 0) {
        // 将YUV的像素数据填充到纹理texture
        RET(SDL_UpdateTexture(_texture, nullptr, data, IMG_W),
            SDL_UpdateTexture);

        // 设置绘制颜色（画笔颜色）
        RET(SDL_SetRenderDrawColor(_renderer,
                                   0, 0, 0, SDL_ALPHA_OPAQUE),
            SDL_SetRenderDrawColor);

        // 设置绘制颜色（画笔颜色）清除渲染目标
        RET(SDL_RenderClear(_renderer), SDL_RenderClear);

        // 拷贝纹理到渲染目标上（默认是window）
        RET(SDL_RenderCopy(_renderer, _texture, nullptr, nullptr), SDL_RenderCopy);

        // 更新所有的渲染操作到屏幕上
        SDL_RenderPresent(_renderer);
    } else {
        // 文件数据已经读取完毕
        killTimer(_timerId);
    }

}
