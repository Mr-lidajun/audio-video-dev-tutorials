#include "demuxer.h"
#include <QDebug>

extern "C" {
#include <libavutil/imgutils.h>
}

#define ERROR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

#define END(func) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        goto end; \
    }

#define RET(func) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        return ret; \
    }

Demuxer::Demuxer() {

}

void Demuxer::demux(const char *inFilename,
                    AudioDecodeSpec &aOut,
                    VideoDecodeSpec &vOut) {
    // 保留参数
    _aOut = &aOut;
    _vOut = &vOut;

    AVPacket *pkt = nullptr;

    // 返回结果
    int ret = 0;

    // 创建解封装上下文、打开文件
    ret = avformat_open_input(&_fmtCtx, inFilename, nullptr, nullptr);
    END(avformat_open_input);

    // 检索流信息
    ret = avformat_find_stream_info(_fmtCtx, nullptr);
    END(avformat_find_stream_info);

    // 打印流信息到控制台
    av_dump_format(_fmtCtx, 0, inFilename, 0);
    fflush(stderr);

    // 初始化音频信息
    ret = initAudioInfo();
    if (ret < 0) {
        goto end;
    }

    // 初始化视频信息
    ret = initVideoInfo();
    if (ret < 0) {
        goto end;
    }

    // 初始化frame
    _frame = av_frame_alloc();
    if (!_frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // 初始化pkt
    pkt = av_packet_alloc();
    pkt->data = nullptr;
    pkt->size = 0;

    // 从输入文件中读取数据
    while (av_read_frame(_fmtCtx, pkt) == 0) {
        if (pkt->stream_index == _aStreamIdx) { // 读取到的是音频数据
            ret = decode(_aDecodeCtx, pkt, &Demuxer::writeAudioFrame);
        } else if (pkt->stream_index == _vStreamIdx) { // 读取到的是视频数据
            ret = decode(_vDecodeCtx, pkt, &Demuxer::writeVideoFrame);
        }
        // 释放pkt内部指针指向的一些额外内存
        av_packet_unref(pkt);

        if (ret < 0) {
            goto end;
        }
    }

    /*
    类方法、静态方法（函数）
    对象方法、动态方法（函数）
    */

    // 刷新缓冲区
//    AVPacket *pkt = _pkt;
//    _pkt = nullptr;
    decode(_aDecodeCtx, nullptr, &Demuxer::writeAudioFrame);
    decode(_vDecodeCtx, nullptr, &Demuxer::writeVideoFrame);

end:
    _aOutFile.close();
    _vOutFile.close();
    avcodec_free_context(&_aDecodeCtx);
    avcodec_free_context(&_vDecodeCtx);
    avformat_close_input(&_fmtCtx);
    av_frame_free(&_frame);
    av_packet_free(&pkt);
    av_freep(&_imgBuf[0]);
}

/**
 * 初始化音频信息
 */
int Demuxer::initAudioInfo() {
    // 初始化解码器
    int ret = initDecoder(&_aDecodeCtx, &_aStreamIdx, AVMEDIA_TYPE_AUDIO);
    RET(initDecoder);

    // 打开文件
    _aOutFile.setFileName(_aOut->filename);
    if (!_aOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _aOut->filename;
        return -1;
    }

    // 保存音频参数
    _aOut->sampleRate = _aDecodeCtx->sample_rate;
    _aOut->sampleFmt = _aDecodeCtx->sample_fmt;
    _aOut->chLayout = _aDecodeCtx->channel_layout;

    return 0;
}

/**
 * 初始化视频信息
 */
int Demuxer::initVideoInfo() {
    // 初始化解码器
    int ret = initDecoder(&_vDecodeCtx, &_vStreamIdx, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);

    // 打开文件
    _vOutFile.setFileName(_vOut->filename);
    if (!_vOutFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << _vOut->filename;
        return -1;
    }

    // 保存视频参数
    _vOut->width = _vDecodeCtx->width;
    _vOut->height = _vDecodeCtx->height;
    _vOut->pixFmt = _vDecodeCtx->pix_fmt;
    // 帧率
    AVRational framerate = av_guess_frame_rate(_fmtCtx,
                                               _fmtCtx->streams[_vStreamIdx],
                                               nullptr);
    _vOut->fps = framerate.num / framerate.den;

    // 创建用于存放一帧解码图片的缓冲区
    ret = av_image_alloc(_imgBuf,
                         _imgLinesizes,
                         _vOut->width,
                         _vOut->height,
                         _vOut->pixFmt, 1);
    RET(av_image_alloc);
    _imgSize = ret;
    return 0;
}

/**
 * 初始化解码器
 * @brief Demuxer::initDecoder
 * @param decodeCtx 解码上下文
 * @param streamIdx 流索引
 * @param type 类型
 * @return
 */
int Demuxer::initDecoder(AVCodecContext **decodeCtx,
                         int *streamIdx,
                         AVMediaType type) {
    // 根据type寻找最合适的流信息
    // 返回值是流索引
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    RET(av_find_best_stream);

    // 检验流
    *streamIdx = ret;
    AVStream *stream = _fmtCtx->streams[*streamIdx];
    if (!stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    // 为当前流找到合适的解码器
    AVCodec *decoder = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decoder not found" << stream->codecpar->codec_id;
        return -1;
    }

    // 初始化解码上下文
    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!decodeCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    // 从流中拷贝参数到解码上下文中
    ret = avcodec_parameters_to_context(*decodeCtx, stream->codecpar);
    RET(avcodec_parameters_to_context);

    // 打开解码器
    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    RET(avcodec_open2);

    return 0;
}

int Demuxer::decode(AVCodecContext *decodeCtx,
                    AVPacket *pkt,
                    void (Demuxer::*func)()) {
    // 发送压缩数据到解码器
    int ret = avcodec_send_packet(decodeCtx, pkt);
    RET(avcodec_send_packet);

    // while (ret >= 0)
    while (true) {
        // 不断从编码器中获取解码后的数据
        ret = avcodec_receive_frame(decodeCtx, _frame);
        // frame中已经没有数据，需要重新发送数据到解码器（send packet）
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        RET(avcodec_receive_frame);

        // 执行写入文件的代码
        (this->*func)();

//        // 将frame的数据写入文件
//        if (decodeCtx->codec->type == AVMEDIA_TYPE_VIDEO) {
//            writeVideoFrame();
//        } else {
//            writeAudioFrame();
//        }
    }
}

void Demuxer::writeVideoFrame() {
    // 将解码后的数据写入文件
    // frame->linesize[0]: 对于视频，以字节为单位的每一行图片的大小
    if (_vDecodeCtx->pix_fmt == AV_PIX_FMT_YUV420P) {
        // 写入Y平面
        _vOutFile.write((char *) _frame->data[0],
                _frame->linesize[0] * _vOut->height);
        // 写入U平面，U平面的高度是整体高度的一半
        _vOutFile.write((char *) _frame->data[1],
                _frame->linesize[1] * _vOut->height >> 1);
        // 写入V平面，V平面的高度是整体高度的一半
        _vOutFile.write((char *) _frame->data[2],
                _frame->linesize[2] * _vOut->height >> 1);
    } else if (_vDecodeCtx->pix_fmt == AV_PIX_FMT_YUV422P) {
        // 代码略
    }
}

void Demuxer::writeAudioFrame() {

}
