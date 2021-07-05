#include "ffmpegs.h"
#include <QtDebug>
#include <QFile>

extern "C" {
    // 重采样相关API
    #include <libavcodec/avcodec.h>
    // 工具相关API（比如错误处理）
    #include <libavutil/avutil.h>
}

// 宏定义，错误处理
#define ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

FFmpegs::FFmpegs()
{

}

/**
 * static表示当前函数为私有的
 * 检查编码器codec是否支持采样格式sample_fmt
 * @brief check_sample_fmt
 * @param coderc
 * @param sample_fmt
 * @return
 */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
//        qDebug() << av_get_sample_fmt_name(*p);
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/**
 * 音频编码
 * @brief encode
 * @param ctx
 * @param frame
 * @param pkt
 * @param outFile
 * @return 返回负数：中途出现了错误
 *         返回0：编码操作正常完成
 */
static int encode(AVCodecContext *ctx,
                  AVFrame *frame,
                  AVPacket *pkt,
                  QFile &outFile) {
    // 发送frame数据到编码器
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_frame error" << errbuf;
        return ret;
    }

    // while (true)
    while (ret >= 0) {
        // 不断从编码器中获取编码后的数据
        ret = avcodec_receive_packet(ctx, pkt);
        // packet中已经没有数据，需要重新发送数据到编码器（send frame）
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // 继续读取数据到frame，然后送到编码器
            return 0;
        } else if (ret < 0) {// 出现了其他错误
            ERROR_BUF(ret);
            qDebug() << "avcodec_receive_packet error" << errbuf;
            return ret;
        }

        // 成功从编码器拿到编码后的数据
        // 将编码后的数据写入文件
        outFile.write((char *) pkt->data, pkt->size);

        // 释放pkt内部的资源
        av_packet_unref(pkt);
    }

    return 0;
}

/**
 * 通过命令行方式进行AAC编码：
 * ffmpeg -ar 44100 -ac 2 -f s16le -i in.pcm -c:a libfdk_aac -b:a 32k -profile:a aac_he_v2 16_out2.aac
 *
 * @brief FFmpegs::aacEncode
 * @param in
 * @param outFilename
 */
void FFmpegs::aacEncode(AudioEncodeSpec &in, const char *outFilename) {
    // 32kbp/s HE_V2

    // 文件
    QFile inFile(in.filename);
    QFile outFile(outFilename);

    // 返回结果
    int ret = 0;

    // 编码器
    AVCodec *codec = nullptr;

    // 编码器上下文
    AVCodecContext *ctx = nullptr;

    // 存放编码前的数据（pcm）
    AVFrame *frame = nullptr;

    // 存放编码后的数据（aac）
    AVPacket *pkt = nullptr;

    // 获取fdk-aac编码器
    codec = avcodec_find_encoder_by_name("libfdk_aac");
    if(!codec) {
       qDebug() << "encoder libfdk_aac not found";
       return;
    }
    qDebug() << codec->name;

    // libfdk_aac对输入数据的要求：采样格式必须是16位整数（AV_SAMPLE_FMT_S16）
    // 检查输入数据的采样格式
    if (!check_sample_fmt(codec, in.sampleFmt)) {
        qDebug() << "Encoder does not support sample format"
                 << av_get_sample_fmt_name(in.sampleFmt);
        return;
    }

    // 创建编码上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    // 设置PCM参数
    ctx->sample_rate = in.sampleRate;
    ctx->sample_fmt = in.sampleFmt;
    ctx->channel_layout = in.chLayout;
    // 比特率:32kbps
    ctx->bit_rate = 32000;
    // 规格
    ctx->profile = FF_PROFILE_AAC_HE_V2;

    // 打开编码器
    AVDictionary *options = nullptr;
    // 开启VBR模式（Variable Bit Rate，可变比特率）
    // 1：质量最低（但是音质仍旧很棒）
//    av_dict_set(&options, "vbr", "1", 0);
    ret = avcodec_open2(ctx, codec, &options);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // 创建AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // 保证frame里面放的都是以样本为单位的整数个样本
    // frame缓冲区中的样本帧数量（由ctx->frame_size决定）
    frame->nb_samples = ctx->frame_size;//frame_size由编码器决定的
    // 采样格式
    frame->format = ctx->sample_fmt;
    // 声道布局
    frame->channel_layout = ctx->channel_layout;

    // 利用nb_samples、format、channel_layout创建AVFrame内部的缓冲区
    ret = av_frame_get_buffer(frame, 0);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_frame_get_buffer error" << errbuf;
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << outFilename;
        goto end;
    }

    // frame->linesize[0]是缓冲区的大小
    // 读取文件数据到frame中
    while ((ret = inFile.read((char *) frame->data[0],
                              frame->linesize[0])) > 0) {
        // 最后一次读取文件数据时，有可能并没有填满frame的缓冲区
        if (ret < frame->linesize[0]) {
            // 声道数
            int chs = av_get_channel_layout_nb_channels(frame->channel_layout);
            // 每个样本的大小
            int bytes = av_get_bytes_per_sample((AVSampleFormat)frame->format);
            // 改为真正有效的样本帧数量
            frame->nb_samples = ret / (chs * bytes);
        }

        // 编码
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }
    }

    // 刷新缓冲区(flush编码器)
    encode(ctx, nullptr, pkt, outFile);

end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    qDebug() << "aac编码完成";
}
