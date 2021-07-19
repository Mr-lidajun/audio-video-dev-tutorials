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

// 输入缓冲区的大小
#define IN_DATA_SIZE 4096

FFmpegs::FFmpegs()
{

}

/*
640*480，yuv420p
---- 640个Y -----
YY............YY |
YY............YY |
YY............YY |
YY............YY
................ 480行
YY............YY
YY............YY |
YY............YY |
YY............YY |
YY............YY |
---- 320个U -----
UU............UU |
UU............UU |
UU............UU |
UU............UU
................ 240行
UU............UU
UU............UU |
UU............UU |
UU............UU |
UU............UU |
---- 320个V -----
VV............VV |
VV............VV |
VV............VV |
VV............VV
................ 240行
VV............VV
VV............VV |
VV............VV |
VV............VV |
VV............VV |


6 * 4，yuv420p
YYYYYY
YYYYYY
YYYYYY
YYYYYY
UUU
UUU
VVV
VVV
*/

static int frameIdx = 0;

static int decode(AVCodecContext *ctx,
                  AVPacket *pkt,
                  AVFrame *frame,
                  QFile &outFile) {
    // 发送压缩数据到解码器
    int ret = avcodec_send_packet(ctx, pkt);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_send_packet error" << errbuf;
        return ret;
    }

    // while (true)
    while (ret >= 0) {
        // 不断从编码器中获取解码后的数据
        ret = avcodec_receive_frame(ctx, frame);
        // frame中已经没有数据，需要重新发送数据到解码器（send packet）
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // 继续读取数据到packet，然后送到解码器
            return 0;
        } else if (ret < 0) { // 出现了其他错误
            ERROR_BUF(ret);
            qDebug() << "avcodec_receive_frame error" << errbuf;
            return ret;
        }

        qDebug() << "解码出第" << ++frameIdx << "帧";

        /*
         * frame->linesize[0] = 640
         * frame->linesize[1] = 320
         * frame->linesize[2] = 320
         */

        // 将解码后的数据写入文件
        // frame->linesize[0]: 对于视频，以字节为单位的每一行图片的大小
        if (ctx->pix_fmt == AV_PIX_FMT_YUV420P) {
            // 写入Y平面
            outFile.write((char *) frame->data[0],
                    frame->linesize[0] * ctx->height);
            // 写入U平面，U平面的高度是整体高度的一半
            outFile.write((char *) frame->data[1],
                    frame->linesize[1] * ctx->height >> 1);
            // 写入V平面，V平面的高度是整体高度的一半
            outFile.write((char *) frame->data[2],
                    frame->linesize[2] * ctx->height >> 1);
        } else if (ctx->pix_fmt == AV_PIX_FMT_YUV422P) {
            // 代码略
        }

//        qDebug() << frame->data[0] << frame->data[1] << frame->data[2];

        /*
         * frame->data[0] 0xd08c400 0x8c400
         * frame->data[1] 0xd0d79c0 0xd79c0
         * frame->data[2] 0xd0ea780 0xea780
         *
         * frame->data[1] - frame->data[0] = 308672 = y平面的大小
         * frame->data[2] - frame->data[1] = 77248 = u平面的大小
         *
         * y平面的大小 640x480*1 = 307200
         * u平面的大小 640x480*0.25 = 76800
         * v平面的大小 640x480*0.25
         */

        // 一帧图片的大小
//        int imgSize = av_image_get_buffer_size(ctx->pix_fmt, ctx->width, ctx->height, 1);
//        // 将解码后的数据写入文件(460800)
//        // qDebug() << imgSize;
//        // outFile.write((char *) frame->data[0], frame->linesize[0]);
//        outFile.write((char *) frame->data[0], imgSize);

    }

    return 0;
}

/**
 * 通过命令行方式进行h264解码：
 * ffmpeg -c:v h264 -i in.h264 out.yuv
 * @brief FFmpegs::h264Decode
 * @param inFilename
 * @param out
 */
void FFmpegs::h264Decode(const char *inFilename, VideoDecodeSpec &out) {
    // 返回结果
    int ret = 0;

    // 用来存放读取的文件数据(h264)
    // 加上AV_INPUT_BUFFER_PADDING_SIZE是为了防止某些优化过的reader一次性读取过多导致越界
    char inDataArray[IN_DATA_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    char *inData = inDataArray;

    // 每次从输入文件中读取的长度（h264）
    int inLen = 0;
    // 是否已经读取到了输入文件的尾部
    int inEnd = 0;

    // 文件
    QFile inFile(inFilename);
    QFile outFile(out.filename);

    // 解码器
    AVCodec *codec = nullptr;
    // 上下文
    AVCodecContext *ctx = nullptr;
    // 解析器上下文
    AVCodecParserContext *parserCtx = nullptr;

    // 存放解码前的数据(h264)
    AVPacket *pkt = nullptr;
    // 存放解码后的数据(yuv)
    AVFrame *frame = nullptr;

    // 获取h264解码器
    // codec = avcodec_find_decoder_by_name("h264");
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        qDebug() << "decoder h264 not found";
        return;
    }

    // 初始化解析器上下文
    parserCtx = av_parser_init(codec->id);
    if (!parserCtx) {
        qDebug() << "av_parser_init error";
        return;
    }

    // 创建上下文
    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        qDebug() << "avcodec_alloc_context3 error";
        goto end;
    }

    // 创建AVPacket
    pkt = av_packet_alloc();
    if (!pkt) {
        qDebug() << "av_packet_alloc error";
        goto end;
    }

    // 创建AVFrame
    frame = av_frame_alloc();
    if (!frame) {
        qDebug() << "av_frame_alloc error";
        goto end;
    }

    // 打开解码器
    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        ERROR_BUF(ret);
        qDebug() << "avcodec_open2 error" << errbuf;
        goto end;
    }

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error:" << inFilename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error:" << out.filename;
        goto end;
    }

    // bug fix
    // https://patchwork.ffmpeg.org/project/ffmpeg/patch/tencent_609A2E9F73AB634ED670392DD89A63400008@qq.com/

    // 读取文件数据
    while ((inLen = inFile.read(inDataArray, IN_DATA_SIZE)) > 0) {
        inData = inDataArray;

        while (inLen > 0) {
            // 经过解析器解析
            ret = av_parser_parse2(parserCtx, ctx,
                                   &pkt->data, &pkt->size,
                                   (uint8_t *)inData, inLen,
                                   AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                ERROR_BUF(ret);
                qDebug() << "av_parser_parse2 error" << errbuf;
                goto end;
            }

            // 跳过已经解析过的数据
            inData += ret;
            // 减去已经解析过的数据大小
            inLen -= ret;

            // 解码，位置不能放在 剩余数据移动操作 的后面
            if (pkt->size > 0 && decode(ctx, pkt, frame, outFile) < 0) {
                goto end;
            }
        }
    }

    // 不需要调用，因为这里pkt的数据不是它自己新产生的，pkt指向的是inData的数据
    // av_packet_unref(pkt);

    // 刷新缓冲区（flush解码器）
    //    pkt->data = NULL;
    //    pkt->size = 0;
    decode(ctx, nullptr, frame, outFile);

    // 赋值输出参数
    out.width = ctx->width;
    out.height = ctx->height;
    out.pixFmt = ctx->pix_fmt;
    // 用framerate.num获取帧率，并不是time_base.den
    out.fps = ctx->framerate.num;

end:
    // 关闭文件
    inFile.close();
    outFile.close();

    // 释放资源
    av_frame_free(&frame);
    av_packet_free(&pkt);
    av_parser_close(parserCtx);
    avcodec_free_context(&ctx);

    qDebug() << "h264解码完成";
}
