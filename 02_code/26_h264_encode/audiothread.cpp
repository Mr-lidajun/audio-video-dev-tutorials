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

    VideoEncodeSpec in;
    in.filename = "D:/Dev/C_CPP/test/wangyushan_crop.yuv";
    in.width = 480;
    in.height = 270;
    in.fps = 30;
    in.pixFmt = AV_PIX_FMT_YUV420P;

    FFmpegs::h264Encode(in, "D:/Dev/C_CPP/test/wangyushan_crop.h264");

    qDebug() << this->currentThread() << "线程正常结束------";
}
