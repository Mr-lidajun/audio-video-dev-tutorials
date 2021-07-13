#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <SDL2/SDL.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置窗口大小
    resize(600, 600);
    _widget = new QWidget(this);
    _widget->setGeometry(50, 50, 512, 512);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void showVersion() {
    SDL_version version;
    SDL_VERSION(&version);
    qDebug() << version.major << version.minor << version.patch;
}


void MainWindow::on_playButton_clicked()
{
    _playThread = new PlayThread((void *)_widget->winId(), this);
//        _playThread = new PlayThread((void *)ui->label->winId(), this);
    _playThread->start();

//    if (_playThread) {// 停止播放
//        _playThread->requestInterruption();
//        _playThread = nullptr;
//        ui->playButton->setText("显示YUV图片");
//    } else {// 开始播放
//        _playThread = new PlayThread((void *)_widget->winId(), this);
////        _playThread = new PlayThread((void *)ui->label->winId(), this);
//        _playThread->start();
//        // 监听线程的结束
//        connect(_playThread, &PlayThread::finished, [this](){
//            _playThread = nullptr;
//            ui->playButton->setText("显示YUV图片");
//        });
//        ui->playButton->setText("停止显示YUV图片");
//    }

}
