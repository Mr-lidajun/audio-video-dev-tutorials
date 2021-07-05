#ifndef RECEIVER_H
#define RECEIVER_H

#include <QObject>

class Receiver : public QObject
{
    Q_OBJECT
public:
    explicit Receiver(QObject *parent = nullptr);

// 自定义槽
public slots:
    int handleExit(int n1, int n2);
};

#endif // RECEIVER_H
