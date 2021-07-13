#include "ffmpegs.h"
#include <QDebug>
#include <QFile>

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#define ERR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof(errbuf));

#define END(ret, func) \
    if (ret < 0) { \
        ERR_BUF(ret); \
        qDebug() << #func <<"error" << errbuf; \
        goto end; \
    }

FFmpegs::FFmpegs(){

}

// yuv420p
// in.frameSize表示一帧的数据大小
//    inData[0] = (uint8_t *) malloc(in.frameSize);
//    inData[1] = inData[0] + 所有Y的大小;
//    inData[2] = inData[0] + 所有Y的大小 + 所有U的大小;
//    // 拷贝像素数据到inData[0]指向的堆空间

//    // Y：表示亮度（Luminance、Luma），占8bit（1字节）
//    inStrides[0] = in.width * in.height * 1;
//    // Cb（U）：蓝色色度分量，占8bit（1字节），Y分量与CbCr分量的总比例是4:1
//    inStrides[1] = inStrides[0] >> 2;
//    // Cr（V）：红色色度分量，占8bit（1字节），Y分量与CbCr分量的总比例是4:1
//    inStrides[2] = inStrides[1];
void FFmpegs::convertRawVideo(RawVideoFile &in, RawVideoFile &out) {
    // 上下文
    SwsContext *ctx = nullptr;

    // 输入、输出缓冲区（一帧的像素数据，指向每一个平面的数据）
    uint8_t *inData[4], *outData[4];
    // 存储的是每一个平面的每一行的大小(linesize)，而不是每一个平面的总大小
    int inStrides[4], outStrides[4];
    // 每一帧图片的大小
    int inFrameSize, outFrameSize;
    // 返回结果
    int ret = 0;
    // 进行到了那一帧
    int frameIdx = 0;
    // 文件
    QFile inFile(in.filename);
    QFile outFile(out.filename);

    // 创建上下文
    ctx = sws_getContext(in.width, in.height, in.format,
                         out.width, out.height, out.format,
                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctx) {
        qDebug() << "sws_getContext error";
        goto end;
    }

    // 输入缓冲区
    ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
    END(ret, av_image_alloc);

    // 输出缓冲区
    ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
    END(ret, av_image_alloc);

    // 打开文件
    if (!inFile.open(QFile::ReadOnly)) {
        qDebug() << "file open error" << in.filename;
        goto end;
    }
    if (!outFile.open(QFile::WriteOnly)) {
        qDebug() << "file open error" << out.filename;
        goto end;
    }

    // 计算每一帧图片的大小
    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    // 转换每一帧的数据
    while (inFile.read((char *) inData[0], inFrameSize) == inFrameSize) {
        // 转换
        sws_scale(ctx, inData, inStrides, 0, in.height,
                  outData, outStrides);
        // 写到输出文件里去（将outData[0]指向的一帧大小写到文件里）
        outFile.write((char *) outData[0], outFrameSize);
        qDebug() << "转换完第" << frameIdx++ << "帧";
    }


end:
    inFile.close();
    outFile.close();
    av_freep(&inData[0]);
    av_freep(&outData[0]);
    sws_freeContext(ctx);
}

void FFmpegs::convertRawVideo(RawVideoFrame &in, RawVideoFrame &out) {
    // 上下文
    SwsContext *ctx = nullptr;

    // 输入、输出缓冲区（一帧的像素数据，指向每一个平面的数据）
    uint8_t *inData[4], *outData[4];
    // 存储的是每一个平面的每一行的大小(linesize)，而不是每一个平面的总大小
    int inStrides[4], outStrides[4];
    // 每一帧图片的大小
    int inFrameSize, outFrameSize;
    // 返回结果
    int ret = 0;

    // 创建上下文
    ctx = sws_getContext(in.width, in.height, in.format,
                         out.width, out.height, out.format,
                         SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!ctx) {
        qDebug() << "sws_getContext error";
        goto end;
    }

    // 输入缓冲区
    ret = av_image_alloc(inData, inStrides, in.width, in.height, in.format, 1);
    END(ret, av_image_alloc);

    // 输出缓冲区
    ret = av_image_alloc(outData, outStrides, out.width, out.height, out.format, 1);
    END(ret, av_image_alloc);

    // 计算每一帧图片的大小
    inFrameSize = av_image_get_buffer_size(in.format, in.width, in.height, 1);
    outFrameSize = av_image_get_buffer_size(out.format, out.width, out.height, 1);

    // 拷贝输入数据（将in.pixels一帧数据拷贝到inData[0]）
    memcpy(inData[0], in.pixels, inFrameSize);

    // 转换一帧的数据
    sws_scale(ctx, inData, inStrides, 0, in.height,
              outData, outStrides);
    // 申请内存空间
    out.pixels = (char *) malloc(outFrameSize);
    // 拷贝输出数据（将outData[0]指向的一帧数据拷贝到out.pixels）
    mempcpy(out.pixels, outData[0], outFrameSize);


end:
    av_freep(&inData[0]);
    av_freep(&outData[0]);
    sws_freeContext(ctx);
}
