#include "ffmpegs.h"
#include <QtDebug>
#include <QFile>

extern "C" {
    // 重采样相关API
    #include <libavcodec/avcodec.h>
    // 工具相关API（比如错误处理）
    #include <libavutil/avutil.h>
    #include <libavutil/imgutils.h>
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
 * 检查编码器codec是否支持像素格式pixFmt
 * @brief check_pix_fmt
 * @param coderc
 * @param pixFmt
 * @return
 */
static int check_pix_fmt(const AVCodec *codec, enum AVPixelFormat pixFmt)
{
    const enum AVPixelFormat *p = codec->pix_fmts;
    while (*p != AV_PIX_FMT_NONE) {
        if (*p == pixFmt) return 1;
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
 * 通过命令行方式进行h264编码：
 * ffmpeg -s 640x480 -pix_fmt yuv420p -i in.yuv -c:v libx264 out.h264
 * ffmpeg -s 640x480 -pix_fmt yuv420p -framerate 30 -i in.yuv -c:v libx264 out.h264
 * -c:v libx264是指定使用libx264作为编码器
 *
 * @brief FFmpegs::h264Encode
 * @param in
 * @param outFilename
 */
void FFmpegs::h264Encode(VideoEncodeSpec &in, const char *outFilename) {

    // 文件
    QFile inFile(in.filename);
    QFile outFile(outFilename);

    // 一帧图片的大小
    int imgSize = av_image_get_buffer_size(in.pixFmt, in.width, in.height, 1);

    // 返回结果
    int ret = 0;

    // 编码器
    AVCodec *codec = nullptr;

    // 编码器上下文
    AVCodecContext *ctx = nullptr;

    // 存放编码前的数据（yuv）
    AVFrame *frame = nullptr;

    // 存放编码后的数据（h264）
    AVPacket *pkt = nullptr;

    // 获取libx264编码器
    codec = avcodec_find_encoder_by_name("libx264");
    if(!codec) {
       qDebug() << "encoder libx264 not found";
       return;
    }

    // libx264对输入数据的要求：像素格式必须是yuv420p (AV_PIX_FMT_YUV420P)
    // 检查输入数据的像素格式
    if (!check_pix_fmt(codec, in.pixFmt)) {
        qDebug() << "unsupported pixel format"
                 << av_get_pix_fmt_name(in.pixFmt);
        return;
    }

    // 创建编码上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        return;
    }

    // 设置yuv参数
    ctx->width = in.width;
    ctx->height = in.height;
    ctx->pix_fmt = in.pixFmt;
    // 设置帧率（1秒显示的帧数是in.fps）
    ctx->time_base = {1, in.fps};

    // 打开编码器
    ret = avcodec_open2(ctx, codec, nullptr);
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

    frame->width = ctx->width;
    frame->height = ctx->height;
    frame->format = ctx->pix_fmt;
    frame->pts = 0;

    // 利用width、height、format创建缓冲区
    ret = av_image_alloc(frame->data, frame->linesize,
                         in.width, in.height, in.pixFmt, 1);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "av_image_alloc error" << errbuf;
        goto end;
    }

    // 利用width、height、pix_fmt创建AVFrame内部的缓冲区
//    ret = av_frame_get_buffer(frame, 0);
//    if (ret < 0) {
//        ERROR_BUF(ret);
//        qDebug() << "av_frame_get_buffer error" << errbuf;
//        goto end;
//    }

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
                              imgSize)) > 0) {

        // 编码
        if (encode(ctx, frame, pkt, outFile) < 0) {
            goto end;
        }

        // 设置帧的序号
        frame->pts++;
    }

    // 刷新缓冲区(flush编码器)
    encode(ctx, nullptr, pkt, outFile);

end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    if (frame) {
        av_freep(&frame->data[0]);
        av_frame_free(&frame);
    }
    av_packet_free(&pkt);
    avcodec_free_context(&ctx);

    qDebug() << "libx264编码完成";
}
