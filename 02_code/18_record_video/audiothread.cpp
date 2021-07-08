#include "audiothread.h"

#include <QDebug>
#include <QFile>
#include <QDateTime>

extern "C" {
    // 设备相关API
    #include <libavdevice/avdevice.h>
    // 格式相关API
    #include <libavformat/avformat.h>
    // 工具相关API（比如错误处理）
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
}

// 宏定义
#ifdef Q_OS_WIN
    // 格式名称
    #define FMT_NAME "dshow"
    // 设备名称
    #define DEVICE_NAME "video=HD Camera"
    // YUV文件名
    #define FILEPATH "D:/Dev/C_CPP/pcm/"
#else
    #define FMT_NAME "avfoundation"
    #define DEVICE_NAME "0"
    #define FILEPATH "/Users/mj/Desktop/"
#endif

// 宏定义，错误处理
#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

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
    qDebug() << this->currentThread() << "线程开始执行------";

    // 录视频逻辑
    // 获取输入格式对象
    AVInputFormat *fmt = av_find_input_format(FMT_NAME);
    if (!fmt) {
        qDebug() << "获取输入格式对象失败" << FMT_NAME;
        return;
    }

    // 格式上下文（将来可以利用上下文操作设备）
    AVFormatContext *ctx = nullptr;

    // 设备参数
    AVDictionary *options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
    av_dict_set(&options, "framerate", "30", 0);

    // 打开设备
    int ret = avformat_open_input(&ctx, DEVICE_NAME, fmt, &options);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avformat_open_input error" << errbuf;
        return;
    }

    // 文件名
    QString filename = FILEPATH;
    filename += QDateTime::currentDateTime().toString("yyyy_MM_dd_HH_mm_ss");
    filename += ".yuv";
    QFile file(filename);

    if (!file.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << filename;
        // 关闭设备
        avformat_close_input(&ctx);
        return;
    }

    // 拿到输入流相关编解码器参数
    AVCodecParameters *params = ctx->streams[0]->codecpar;
    // 像素格式
    AVPixelFormat pixFmt = (AVPixelFormat) params->format;
    qDebug() << pixFmt << params->width << params->height;
    qDebug() << av_pix_fmt_desc_get(pixFmt)->name;

    // 计算一帧的字节大小 - 方案1，通过内置函数计算
    int imageSize = av_image_get_buffer_size(
                        pixFmt, // 像素格式
                        params->width,// 每一帧的宽度
                        params->height,// 每一帧的高度
                        1);

    // 一个像素的字节大小
//    int pixSize = av_get_bits_per_pixel(av_pix_fmt_desc_get(pixFmt)) >> 3;
    // 计算一帧的字节大小 - 方案2
//    int imageSize = params->width * params->height * pixSize;

    qDebug() << "imageSize:" << imageSize;

    // 数据包
    //AVPacket pkt; // AVPacket对象在栈空间
    // AVPacket对象在堆空间，pkt指针在栈空间
    AVPacket *pkt = av_packet_alloc();
    while (!isInterruptionRequested()) {
        // 不断采集数据
        ret = av_read_frame(ctx, pkt);
        if (ret == 0) {// 读取成功
            // 将数据写入文件
            file.write((const char *)pkt->data, imageSize);
            /*
             这里要使用imageSize，而不是pkt->size。
             pkt->size有可能比imageSize大（比如在Mac平台），
             使用pkt->size会导致写入一些多余数据到YUV文件中，
             进而导致YUV内容无法正常播放
            */

            // windows: 614400
            // mac: 615680
            // qDebug() << pkt->size;

            // 释放资源
            av_packet_unref(pkt);
        } else if (ret == AVERROR(EAGAIN)) {// 资源临时不可用
            continue;
        } else {// 其他错误
            ERROR_BUF(ret);
            qDebug() << "av_read_frame" << errbuf << ret;
            break;
        }

    }

    // 释放资源
    av_packet_free(&pkt);

    // 关闭文件
    file.close();

    // 关闭设备
    avformat_close_input(&ctx);

    qDebug() << this->currentThread() << "线程正常结束------";
}
