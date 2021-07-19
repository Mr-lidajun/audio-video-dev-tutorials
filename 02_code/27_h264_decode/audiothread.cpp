#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"

extern "C" {
#include <libavutil/imgutils.h>
}

AudioThread::AudioThread(QObject *parent) : QThread(parent)
{
    // 当监听到线程结束时（finished），就调用deleteLater回收内存
    connect(this, &AudioThread::finished,
            this, &AudioThread::deleteLater);
}

// 析构函数
AudioThread::~AudioThread() {
    // 断开所有的连接
    disconnect();
    // 内存回收之前，正常结束线程
    requestInterruption();
    // 安全退出
    quit();
    wait();
    qDebug() << this << "析构（内存被回收）";
}

// 当线程启动的时候（start），就会自动调用run函数
// run函数中的代码是在子线程中执行的
// 耗时操作应该放在run函数中
void AudioThread::run() {

    VideoDecodeSpec out;
    out.filename = "D:/Dev/C_CPP/test/27_dragon_ball_8.yuv";
    FFmpegs::h264Decode("D:/Dev/C_CPP/test/26_dragon_ball_8.h264", out);

    // 宽度
    qDebug() << "width：" << out.width;
    // 高度
    qDebug() << "height：" << out.height;
    // 帧率
    qDebug() << "fps：" << out.fps;
    // 像素格式
    qDebug() << "像素格式：" << av_get_pix_fmt_name(out.pixFmt);

    qDebug() << this->currentThread() << "线程正常结束------";
}
