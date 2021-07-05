#include "receiver.h"
#include <QDebug>

Receiver::Receiver(QObject *parent) : QObject(parent)
{

}

// 实现槽函数，编写处理信号的代码
int Receiver::handleExit(int n1, int n2)
{
    qDebug() << "Receiver::handleExit" << n1 << n2;
    return n1 + n2;
}
