#include "audiothread.h"

#include <QDebug>
#include "demuxer.h"

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

    AudioDecodeSpec aOut;
    aOut.filename = "D:/Dev/C_CPP/test/28_out_dragon_ball.pcm";

    VideoDecodeSpec vOut;
    vOut.filename = "D:/Dev/C_CPP/test/28_out_dragon_ball.yuv";

    Demuxer().demux("D:/Dev/C_CPP/test/dragon_ball_8.mkv", aOut, vOut);

    qDebug() << aOut.sampleRate
             << av_get_channel_layout_nb_channels(aOut.chLayout)
             << av_get_sample_fmt_name(aOut.sampleFmt);

    qDebug() << vOut.width << vOut.height
             << vOut.fps << av_get_pix_fmt_name(vOut.pixFmt);

    qDebug() << this->currentThread() << "线程正常结束------";
}
