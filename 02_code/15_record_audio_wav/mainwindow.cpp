#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <audiothread.h>
#include <QTime>
#include <QtDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 设置窗口大小
    resize(300, 200);

    // 初始化时间
    onTimeChanged(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onTimeChanged(unsigned long long ms) {
    QTime time(0, 0, 0, 0);
    QString text = time.addMSecs(ms).toString("hh:mm:ss");
    ui->timeLabel->setText(text);
}

void MainWindow::on_audioButton_clicked()
{
    if (!_audioThread) {// 点击了"开始录音"
        // 开启线程
        _audioThread = new AudioThread(this);
        _audioThread->start();

        connect(_audioThread, &AudioThread::timeChanged,
                this, &MainWindow::onTimeChanged);

        connect(_audioThread, &AudioThread::finished,
            [this]() {// 线程结束
            _audioThread = nullptr;
            ui->audioButton->setText("开始录音");
        });

        //设置按钮文字
        ui->audioButton->setText("结束录音");
    } else {// 点击了"结束录音"
        //结束线程
        _audioThread->requestInterruption();
        _audioThread = nullptr;

        // 设置按钮文字
        ui->audioButton->setText("开始录音");
    }

}
