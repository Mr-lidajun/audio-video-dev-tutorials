#include "videoplayer.h"
#include <QDebug>

int VideoPlayer::initVideoInfo() {
    // 初始化解码器
    int ret = initDecoder(&_vDecodeCtx, &_vStream, AVMEDIA_TYPE_VIDEO);
    RET(initDecoder);

    return 0;
}

void VideoPlayer::addVideoPkt(AVPacket &pkt) {

}