#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <list>
#include <SDL2/SDL.h>
#include "condmutex.h"

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

    void on_productBtn_clicked();

private:
    Ui::MainWindow *ui;
//    /** 互斥锁 */
//    SDL_mutex *_mutex = nullptr;
//    /** 条件变量 */
//    SDL_cond *_cond = nullptr;

    CondMutex *_mutex = nullptr;
    std::list<QString> *_list;
    int _index = 0;

    /** 消费者 */
    void consume(QString name);
    /** 生产者 */
    void produce(QString name);
};
#endif // MAINWINDOW_H
