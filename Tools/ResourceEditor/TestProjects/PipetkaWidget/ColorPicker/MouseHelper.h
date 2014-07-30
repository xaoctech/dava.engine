#ifndef MOUSEHELPER_H
#define MOUSEHELPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>

class MouseHelper
    : public QObject
{
    Q_OBJECT

signals:

public:
    explicit MouseHelper( QWidget *w );
    ~MouseHelper();

private:
    bool eventFilter( QObject *obj, QEvent *e ) override;

    void enterEvent( QEvent * event );
    void leaveEvent( QEvent * event );
    void mouseMoveEvent( QMouseEvent * event );
    void mousePressEvent( QMouseEvent * event );
    void mouseReleaseEvent( QMouseEvent * event );

    QPointer<QWidget> w;
    QPoint pos;
    bool isHover;
    bool isPressed;
};

#endif // MOUSEHELPER_H
