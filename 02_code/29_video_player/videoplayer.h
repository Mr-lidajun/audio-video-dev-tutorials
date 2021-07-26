#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>

#define ERROR_BUF \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));

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


private:
    /******** 其他 ********/
    /** 当前的状态 */
    State _state = Stopped;
    /** 文件名 */
    const char *_filename;
    /** 改变状态 */
    void setState(State state);

signals:
    void stateChanged(VideoPlayer *player);

};

#endif // VIDEOPLAYER_H
