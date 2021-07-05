#ifndef SENDER_H
#define SENDER_H

#include <QObject>

class Sender : public QObject
{
    Q_OBJECT
public:
    explicit Sender(QObject *parent = nullptr);

    // 自定义信号
signals:
    int exit(int n1, int n2);
};

#endif // SENDER_H
