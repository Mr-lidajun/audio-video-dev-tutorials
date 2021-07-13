#ifndef YUVPLAYER_H
#define YUVPLAYER_H

#include <SDL2/SDL.h>
#include <QWidget>
#include <QFile>

extern "C" {
#include <libavutil/avutil.h>
}

typedef  struct {
    const char *filename;
    int width;
    int height;
    AVPixelFormat pixeFormat;
    int fps;
} Yuv;

class YuvPlayer : public QWidget
{
    Q_OBJECT
public:
    // 播放器状态
    typedef enum {
        Stopped = 0,
        Playing,
        Paused,
        Finished
    } State;

    explicit YuvPlayer(QWidget *parent = nullptr);
    ~YuvPlayer();

    void play();
    void pause();
    void stop();
    bool isPlaying();

    void setYuv(Yuv &yuv);

    State getState();

private:
    // 窗口
    SDL_Window *_window = nullptr;
    // 渲染上下文
    SDL_Renderer *_renderer = nullptr;
    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *_texture = nullptr;
    // 文件
    QFile _file;
    // 播放器状态
    State _state = Stopped;
    // 定时器ID
    int _timerId = 0;
    // 定时器函数，重写这个虚函数
    void timerEvent(QTimerEvent *event);
    // 非引用，会拷贝一份，如果是引用，存在引用内容被销毁的危险
    Yuv _yuv;

signals:

};

#endif // YUVPLAYER_H
