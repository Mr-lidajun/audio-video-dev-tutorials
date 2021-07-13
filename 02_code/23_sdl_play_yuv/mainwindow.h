#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <SDL2/SDL.h>

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

private:
    Ui::MainWindow *ui;

    QWidget *_widget;
    // 窗口
    SDL_Window *_window;
    // 渲染上下文
    SDL_Renderer *_renderer;
    // 纹理（直接跟特定驱动程序相关的像素数据）
    SDL_Texture *_texture;
    // 文件
    QFile _file;
    // 定时器ID
    int _timerId;
    // 定时器函数，重写这个虚函数
    void timerEvent(QTimerEvent *event);
};
#endif // MAINWINDOW_H
