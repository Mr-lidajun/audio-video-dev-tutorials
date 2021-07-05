#ifndef SENDER2_H
#define SENDER2_H

#include <QObject>

class Sender2 : public QObject
{
    Q_OBJECT
public:
    explicit Sender2(QObject *parent = nullptr);

    // 自定义信号
signals:
    void exit2(bool checked);
};

#endif // SENDER2_H
