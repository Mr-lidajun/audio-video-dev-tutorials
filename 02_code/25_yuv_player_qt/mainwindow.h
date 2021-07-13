#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include "yuvplayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //这是虚函数,重写这个虚函数,当你按下窗口右上角的"×"时,就会调用你所重写的此函数.
    void closeEvent(QCloseEvent *event);

private slots:
    void on_playButton_clicked();

    void on_stopButton_clicked();

private:
    Ui::MainWindow *ui;

    YuvPlayer *_player = nullptr;
};
#endif // MAINWINDOW_H
