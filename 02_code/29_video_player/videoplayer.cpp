#include "videoplayer.h"
#include <thread>
#include <QDebug>

#pragma mark - 构造、析构
VideoPlayer::VideoPlayer(QObject *parent) : QObject(parent)
{
//    _aPktList = new std::list<AVPacket>();
//    _vPktList = new std::list<AVPacket>();
}

VideoPlayer::~VideoPlayer() {

}

#pragma mark - 公共方法
void VideoPlayer::play() {
    if (_state == Playing) return;
    // 状态可能是：暂停、停止、正常完毕

    // 开启线程：读取文件
    std::thread([this]() {
        readFile();
    }).detach();

    // 改变状态
    setState(Playing);
}

void VideoPlayer::pause() {
    if (_state != Playing) return;
    // 状态可能是：正在播放

    // 改变状态
    setState(Paused);
}

void VideoPlayer::stop() {
    if (_state == Stopped) return;
    // 状态可能是：正在播放、暂停、正常完毕

    // 改变状态
    setState(Stopped);

    // 通知外界
    emit stateChanged(this);
}

bool VideoPlayer::isPlaying() {
    return _state == Playing;
}

VideoPlayer::State VideoPlayer::getState() {
    return _state;
}

void VideoPlayer::setFilename(QString &filename) {
    _filename = filename.toUtf8().data();
    qDebug() << "filename" << _filename;
}

int64_t VideoPlayer::getDuration() {
//    return _fmtCtx ? _fmtCtx->duration : 0;
     return _fmtCtx ? round(_fmtCtx->duration / 1000000.0) : 0;
}

#pragma mark - 私有方法
void VideoPlayer::readFile() {
    // 返回结果
    int ret = 0;

    // 创建解封装上下文、打开文件
    ret = avformat_open_input(&_fmtCtx, _filename, nullptr, nullptr);
    END(avformat_open_input);

    // 检索流信息
    ret = avformat_find_stream_info(_fmtCtx, nullptr);
    END(avformat_find_stream_info);

    // 打印流信息到控制台
    av_dump_format(_fmtCtx, 0, _filename, 0);
    fflush(stderr);

    // 初始化音频信息
    _hasAudio = initAudioInfo() >= 0;
    // 初始化视频信息
    _hasVideo = initVideoInfo() >= 0;
    if (!_hasAudio && !_hasVideo) {
        return;
    }

    // 到此为止，初始化完毕
    emit initFinished(this);

    // 改变状态
    setState(Playing);

    // 从输入文件中读取数据
    AVPacket pkt;

    // 从输入文件中读取数据
    while (true) {
        ret = av_read_frame(_fmtCtx, &pkt);
        if (ret == 0) {
            if (pkt.stream_index == _aStream->index) { // 读取到的是音频数据
                addAudioPkt(pkt);
            } else if (pkt.stream_index == _vStream->index) { // 读取到的是视频数据
                addVideoPkt(pkt);
            }
        } else {
            continue;
        }
    }

end:
    avcodec_free_context(&_aDecodeCtx);
    avcodec_free_context(&_vDecodeCtx);
    avformat_close_input(&_fmtCtx);
}


void VideoPlayer::setState(State state) {
    if (state == _state) return;

    _state = state;

    emit stateChanged(this);
}

/**
 * 初始化解码器
 * @brief Demuxer::initDecoder
 * @param decodeCtx 解码上下文
 * @param AVStream 流
 * @param type 类型
 * @return
 */
int VideoPlayer::initDecoder(AVCodecContext **decodeCtx,
                         AVStream **stream,
                         AVMediaType type) {
    // 根据type寻找最合适的流信息
    // 返回值是流索引
    int ret = av_find_best_stream(_fmtCtx, type, -1, -1, nullptr, 0);
    RET(av_find_best_stream);

    // 检验流
    int streamIdx = ret;
    *stream = _fmtCtx->streams[streamIdx];
    if (!*stream) {
        qDebug() << "stream is empty";
        return -1;
    }

    // 为当前流找到合适的解码器
    AVCodec *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
    if (!decoder) {
        qDebug() << "decoder not found" << (*stream)->codecpar->codec_id;
        return -1;
    }

    // 初始化解码上下文
    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!decodeCtx) {
        qDebug() << "avcodec_alloc_context3 error";
        return -1;
    }

    // 从流中拷贝参数到解码上下文中
    ret = avcodec_parameters_to_context(*decodeCtx, (*stream)->codecpar);
    RET(avcodec_parameters_to_context);

    // 打开解码器
    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    RET(avcodec_open2);

    return 0;
}
