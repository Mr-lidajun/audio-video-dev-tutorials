#include "yuvplayer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

/**定义条件宏*/
#define RET(judge, func) \
    if (judge) { \
        qDebug() << #func << "error" << SDL_GetError(); \
        return; \
    }

static const std::map<AVPixelFormat, SDL_PixelFormatEnum>
PIXEL_FORMAT_MAP = {
    {AV_PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
    {AV_PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2 },
    {AV_PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN }
};

YuvPlayer::YuvPlayer(QWidget *parent) : QWidget(parent) {
    // 创建一个窗口
    _window = SDL_CreateWindowFrom((void *) winId());
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
}

// 析构函数
YuvPlayer::~YuvPlayer() {
    _file.close();
    SDL_DestroyTexture(_texture);
    SDL_DestroyRenderer(_renderer);
    SDL_DestroyWindow(_window);
}

void YuvPlayer::play() {
    // 开启定时器，1000 / _yuv.fps表示一帧需要多少时间
    _timerId = startTimer(1000 / _yuv.fps);
    _state = YuvPlayer::Playing;
}

void YuvPlayer::pause() {
    if (_timerId) {// 非0为真
        killTimer(_timerId);
    }
    _state = YuvPlayer::Paused;
}

void YuvPlayer::stop() {
    if (_timerId) {// 非0为真
        killTimer(_timerId);
    }
    _state = YuvPlayer::Stopped;
}

bool YuvPlayer::isPlaying() {
    return _state == YuvPlayer::Playing;
}

YuvPlayer::State YuvPlayer::getState() {
    return _state;
}

void YuvPlayer::setYuv(Yuv &yuv) {
    _yuv = yuv;

    // 创建纹理
    _texture = SDL_CreateTexture(_renderer,
                                PIXEL_FORMAT_MAP.find(yuv.pixeFormat)->second,
                                SDL_TEXTUREACCESS_STREAMING,
                                yuv.width, yuv.height);
    RET(!_texture, SDL_CreateTexture);

    // 打开文件
    _file.setFileName(yuv.filename);
    if (!_file.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << yuv.filename;
    }
}

// 每隔一段时间就会调用
void YuvPlayer::timerEvent(QTimerEvent *event) {
    // 存储格式为：I420(yuv420p)，1个像素平均占用12bit（1.5字节）
    // 一帧图片的大小
    //int imgSize = width * height * 1.5;
    int imgSize = av_image_get_buffer_size(_yuv.pixeFormat,
                                           _yuv.width, _yuv.height, 1);
    char data[imgSize];
    if (_file.read(data, imgSize) > 0) {
        // 将YUV的像素数据填充到纹理texture
        RET(SDL_UpdateTexture(_texture, nullptr, data, _yuv.width),
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
