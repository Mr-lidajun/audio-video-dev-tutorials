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
    // 4次重采样
    // 44100_s16le_2 -> 48000_f32le_2 -> 48000_s32le_1 -> 44100_s16le_2

    ResampleAudioSpec ras1;
    ras1.filename = "D:/Dev/C_CPP/test/44100_s16le_2.pcm";
    ras1.sampleFmt = AV_SAMPLE_FMT_S16;
    ras1.sampleRate = 44100;
    // 双声道
    ras1.chLayout = AV_CH_LAYOUT_STEREO;

    ResampleAudioSpec ras2;
    ras2.filename = "D:/Dev/C_CPP/test/48000_f32le_1.pcm";
    ras2.sampleFmt = AV_SAMPLE_FMT_FLT;
    ras2.sampleRate = 48000;
    // 单声道
    ras2.chLayout = AV_CH_LAYOUT_MONO;

    ResampleAudioSpec ras3;
    ras3.filename = "D:/Dev/C_CPP/test/48000_s32le_1.pcm";
    ras3.sampleFmt = AV_SAMPLE_FMT_S32;
    ras3.sampleRate = 48000;
    ras3.chLayout = AV_CH_LAYOUT_MONO;

    ResampleAudioSpec ras4 = ras1;
    ras4.filename = "D:/Dev/C_CPP/test/44100_s16le_2_new.pcm";

    FFmpegs::resampleAudio(ras1, ras2);
    FFmpegs::resampleAudio(ras2, ras3);
    FFmpegs::resampleAudio(ras3, ras4);

    qDebug() << this->currentThread() << "线程正常结束------";
}
