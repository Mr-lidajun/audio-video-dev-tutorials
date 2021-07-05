#include "audiothread.h"

#include <QDebug>
#include "ffmpegs.h"

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

    AudioEncodeSpec out;
    out.filename = "D:/Dev/C_CPP/test/out.pcm";
    FFmpegs::aacDecode("D:/Dev/C_CPP/test/out.aac", out);

    // 采样率：44100
    qDebug() << "采样率：" << out.sampleRate;
    // 采样格式：s16
    qDebug() << "采样格式：" << av_get_sample_fmt_name(out.sampleFmt);
    // 声道数：2
    qDebug() << "声道数：" << av_get_channel_layout_nb_channels(out.chLayout);

    qDebug() << this->currentThread() << "线程正常结束------";
}
