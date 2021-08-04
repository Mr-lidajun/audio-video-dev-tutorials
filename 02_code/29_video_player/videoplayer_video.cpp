#include "videoplayer.h"
#include <QDebug>
#include <thread>

extern "C" {
#include <libavutil/imgutils.h>
}

int VideoPlayer::initVideoInfo() {
    // 初始化解码器
    int ret = initDecoder(&_vDecodeCtx, &_vStream, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);

    // 初始化像素格式转换
    initSws();
    RET(initSws);

    // 开启新的线程去解码视频数据
    std::thread([this](){
        decodeVideo();
    }).detach();

    return 0;
}

int VideoPlayer::initSws() {
    int inW = _vDecodeCtx->width;
    int inH = _vDecodeCtx->height;

    // 输出frame的参数
    _vSwsOutSpec.width = inW >> 4 << 4;// 16的倍数
    _vSwsOutSpec.height = inH >> 4 << 4;
    _vSwsOutSpec.pixFmt = AV_PIX_FMT_RGB24;
    // 存储格式为：I420(yuv420p)，1个像素平均占用12bit（1.5字节）
    // 一帧图片的大小
    //size = width * height * 1.5;
    _vSwsOutSpec.size = av_image_get_buffer_size(
                            _vSwsOutSpec.pixFmt,
                            _vSwsOutSpec.height,
                            _vSwsOutSpec.width, 1);

    // 初始化像素格式转换的上下文
    _vSwsCtx = sws_getContext(inW,
                              inH,
                              _vDecodeCtx->pix_fmt,

                              _vSwsOutSpec.width,
                              _vSwsOutSpec.height,
                              _vSwsOutSpec.pixFmt,

                              SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!_vSwsCtx) {
        qDebug() << "sws_getContext error";
        return -1;
    }

    // 初始化视频像素格式转换的输入frame
    _vSwsInFrame = av_frame_alloc();
    if (!_vSwsInFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    // 初始化像素格式转换的输出frame
    _vSwsOutFrame = av_frame_alloc();
    if (!_vSwsOutFrame) {
        qDebug() << "av_frame_alloc error";
        return -1;
    }

    // _vSwsOutFrame的data[0]指向的内存空间
    int ret = av_image_alloc(_vSwsOutFrame->data,
                             _vSwsOutFrame->linesize,
                             _vSwsOutSpec.width,
                             _vSwsOutSpec.height,
                             _vSwsOutSpec.pixFmt,
                             1);
    RET(av_image_alloc);

    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt) {
    _vMutex.lock();
    _vPktList.push_back(pkt);
    _vMutex.signal();
    _vMutex.unlock();
}


void VideoPlayer::clearVideoPktList() {
    _vMutex.lock();
    for (AVPacket &pkt : _vPktList) {
        av_packet_unref(&pkt);
    }
    _vPktList.clear();
    _vMutex.unlock();
}

void VideoPlayer::freeVideo() {
    clearVideoPktList();
    avcodec_free_context(&_vDecodeCtx);
}

void VideoPlayer::decodeVideo() {
    while (true) {
        _vMutex.lock();

        if (_vPktList.empty()) {
            _vMutex.unlock();
            continue;
        }

        // 取出头部的视频包
        // 拷贝构造
        AVPacket pkt = _vPktList.front();
        _vPktList.pop_front();
        _vMutex.unlock();

        // 发送压缩数据到解码器
        int ret = avcodec_send_packet(_vDecodeCtx, &pkt);
        // 释放pkt
        av_packet_unref(&pkt);
        CONTINUE(avcodec_send_packet);

        while (true) {
            // 获取解码后的数据
            ret = avcodec_receive_frame(_vDecodeCtx, _vSwsInFrame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            } else BREAK(avcodec_receive_frame);

            // TODO 假停顿，1000/30
            SDL_Delay(33);

            // 像素格式的转换
            // _vSwsInFrame(yuv420p) -> _vSwsOutFrame(rgb24)
            sws_scale(_vSwsCtx,
                      _vSwsInFrame->data, _vSwsInFrame->linesize,
                      0, _vDecodeCtx->height,
                      _vSwsOutFrame->data, _vSwsOutFrame->linesize);

//            qDebug() << _vSwsOutFrame->data[0];

            // 发出信号
            emit frameDecoded(this, _vSwsOutFrame->data[0], _vSwsOutSpec);
        }

    }
}
