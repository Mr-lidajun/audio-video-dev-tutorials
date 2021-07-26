#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

#define ERROR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

#define END(func) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        setState(Stopped); \
        emit playFailed(this); \
        goto end; \
    }

#define RET(func) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        return ret; \
    }

#define CODE(func, code) \
    if (ret < 0) { \
        ERROR_BUF; \
        qDebug() << #func << "error" << errbuf; \
        code; \
    }

/**
 * 预处理视频数据（不负责显示、渲染视频）
 */
class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    // 状态
    typedef enum {
        Stopped = 0,
        Playing,
        Paused
    } State;

    explicit VideoPlayer(QObject *parent = nullptr);
    // 析构函数
    ~VideoPlayer();

    /** 播放 */
    void play();
    /** 暂停 */
    void pause();
    /** 停止 */
    void stop();
    /** 是否正在播放中 */
    bool isPlaying();
    /** 获取当前的状态 */
    State getState();
    /** 设置文件名 */
    void setFilename(QString &filename);
    /** 获取总时长（单位是微秒，1秒=10^3毫秒=10^6微秒） */
    /** 获取总时长（单位是秒） */
    int64_t getDuration();


private:
    /******** 音频相关 ********/
    typedef struct {
        int sampleRate;
        AVSampleFormat sampleFmt;
        int chLayout;
        int chs;
        int bytesPerSampleFrame;
    } AudioSwrSpec;

    /** 解码上下文 */
    AVCodecContext *_aDecodeCtx = nullptr;
    /** 流 */
    AVStream *_aStream = nullptr;

    /** 初始化音频信息 */
    int initAudioInfo();
    /** 初始化音频重采样 */
    int initSwr();
    /** 初始化SDL */
    int initSDL();
    /** 添加数据包到音频包列表中 */
    void addAudioPkt(AVPacket &pkt);
    /** 是否有音频流 */
    bool _hasAudio = false;

    /******** 视频相关 ********/
    /** 解码上下文 */
    AVCodecContext *_vDecodeCtx = nullptr;
    /** 流 */
    AVStream *_vStream = nullptr;
    /** 是否有视频流 */
    bool _hasVideo = false;

    /** 初始化视频信息 */
    int initVideoInfo();

    /******** 其他 ********/
    /** 解封装上下文 */
    AVFormatContext *_fmtCtx = nullptr;
    /** 当前的状态 */
    State _state = Stopped;
    /** 文件名 */
    const char *_filename;
    /** 初始化解码器和解码上下文 */
    int initDecoder(AVCodecContext **decodeCtx,
                    AVStream **stream,
                    AVMediaType type);
    /** 改变状态 */
    void setState(State state);
    /** 读取文件数据 */
    void readFile();
    /** 严重错误 */
    void fataError();

signals:
    void stateChanged(VideoPlayer *player);
    void initFinished(VideoPlayer *player);
    void playFailed(VideoPlayer *player);
};

#endif // VIDEOPLAYER_H
