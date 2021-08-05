#include "videowidget.h"
#include <QDebug>
#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    // 设置背景色
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
}

void VideoWidget::paintEvent(QPaintEvent *event) {
    if (!_image) return;

    // 将图片绘制到当前组件上
    QPainter(this).drawImage(_rect, *_image);
}

void VideoWidget::onPlayerStateChanged(VideoPlayer *player) {
    if (player->getState() != VideoPlayer::Stopped) return;

    freeImage();
    // 刷新
    update();
}

void VideoWidget::onPlayerFrameDecoded(VideoPlayer *player,
                                       uint8_t *data,
                                       VideoPlayer::VideoSwsSpec &spec) {
    freeImage();

    // 创建新的图片
    if (data != nullptr) {
        _image = new QImage(data,
                            spec.width, spec.height,
                            QImage::Format_RGB888);
        // 计算最终尺寸
        // 组件的尺寸
        int w = width();
        int h = height();

        // 计算rect
        int dx = 0;
        int dy = 0;
        int dw = spec.width;
        int dh = spec.height;

        // 计算目标尺寸
        if (dw > w || dh > h) {// 缩放
            // dw/dh > w/h
            if (dw * h > w * dh) { // 视频的宽高比 > 播放器的宽高比
                /**
                    w    dw
                    -- = --
                    dh   dh
                */
                // 已知缩放视频的宽度（w），根据视频的宽高比（dw/dh）,求出缩放视频的高度(dh)
                dh = w * dh / dw;
                dw = w;
            } else {
                dw = h * dw / dh;
                dh = h;
            }
        }

        // 居中
        dx = (w - dw) >> 1;
        dy = (h - dh) >> 1;

        _rect = QRect(dx, dy, dw, dh);
    }

    // 刷新
    update();
}

void VideoWidget::freeImage() {
    if (_image) {
        delete _image;
        _image = nullptr;
    }
}
