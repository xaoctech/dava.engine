#ifndef EYEDROPPER_H
#define EYEDROPPER_H

#include <QObject>


class EyeDropper
    : public QObject
{
    Q_OBJECT

public:
    explicit EyeDropper(QObject *parent = NULL);
    ~EyeDropper();

private:
};

#endif // EYEDROPPER_H
