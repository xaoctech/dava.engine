#ifndef EYEDROPPER_H
#define EYEDROPPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>


class MouseHelper;

class EyeDropper
    : public QObject
{
    Q_OBJECT

    typedef QList< QPointer< QWidget > > ShadesList;

public:
    explicit EyeDropper(QObject *parent = NULL);
    ~EyeDropper();

    void CreateShade();

private:
    QPointer<QWidget> shade;
    QPointer<MouseHelper> mouse;
};


#endif // EYEDROPPER_H
