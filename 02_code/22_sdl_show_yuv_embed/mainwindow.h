#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "playthread.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_playButton_clicked();

private:
    Ui::MainWindow *ui;
    PlayThread *_playThread = nullptr;
    QWidget *_widget;
};
#endif // MAINWINDOW_H
