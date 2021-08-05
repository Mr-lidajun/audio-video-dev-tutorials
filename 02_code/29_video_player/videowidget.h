#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include "videoplayer.h"

/**
 * 显示（渲染）视频
 */
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);

public slots:
    void onPlayerFrameDecoded(VideoPlayer *player,
                              uint8_t *data,
                              VideoPlayer::VideoSwsSpec &spec);
    void onPlayerStateChanged(VideoPlayer *player);

private:
    QImage *_image = nullptr;
    QRect _rect;
    void paintEvent(QPaintEvent *event) override;

    /** 释放之前的图片 */
    void freeImage();

signals:

};

#endif // VIDEOWIDGET_H
