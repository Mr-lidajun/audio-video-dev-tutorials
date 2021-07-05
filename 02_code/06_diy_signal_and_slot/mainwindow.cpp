#include "mainwindow.h"
#include "sender.h"
#include "sender2.h"
#include "receiver.h"

#include <QtDebug>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QPushButton *btn = new QPushButton;
    btn->setText("按钮");
    btn->setFixedSize(100, 40);
    btn->setParent(this);

    //    connect(btn, &QPushButton::clicked, [](){
    //        qDebug() << "点击了按钮";
    //    });

    connect(btn, &QPushButton::clicked, this, &MainWindow::handleClick);

    Sender *sender = new Sender;
    Sender2 *sender2 = new Sender2;
    Receiver *receiver = new Receiver;

    //connect(btn, &QPushButton::clicked, sender2, &Sender2::exit2);

//    connect(sender, &Sender::exit,
//            receiver, &Receiver::handleExit);

//    qDebug() << emit sender->exit(10, 20);

    //使用Lambda处理信号
//    connect(sender, &Sender::exit, [](int n1, int n2){
//        qDebug() << "Lambda" << n1 << n2;
//    });

    //emit sender->exit(10, 20);

    connect(sender2, &Sender2::exit2, [](){
        qDebug() << "Lambda";
    });
    //emit sender->exit2( true);

    delete sender;
    delete sender2;
    delete receiver;
}

void MainWindow::handleClick()
{
    qDebug() << "点击了按钮-handleClick";
}

MainWindow::~MainWindow()
{
}

