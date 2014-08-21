#ifndef EYEDROPPER_H
#define EYEDROPPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>


class EyeDropper
    : public QObject
{
    Q_OBJECT

    typedef QList< QPointer< QWidget > > ShadesList;

public:
    explicit EyeDropper(QObject *parent = NULL);
    ~EyeDropper();

private:
    void CreateShades();
    QWidget * CreateShade( int screen ) const;

    ShadesList shades;
};

#endif // EYEDROPPER_H
