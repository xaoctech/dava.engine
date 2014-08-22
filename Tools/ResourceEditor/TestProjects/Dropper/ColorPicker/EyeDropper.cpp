#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDebug>

#include "../Helpers/MouseHelper.h"


EyeDropper::EyeDropper(QObject *parent)
    : QObject(parent)
{
}

EyeDropper::~EyeDropper()
{
    delete shade;
}

void EyeDropper::Exec()
{
    CreateShade();
}

void EyeDropper::OnMouseMove()
{
    emit moved(GetPixel());
}

void EyeDropper::OnClicked()
{
    emit clicked(GetPixel());
}

void EyeDropper::CreateShade()
{
    QDesktopWidget *desktop = QApplication::desktop();

    const int n = desktop->screenCount();
    QRect screenRc;
    for (int i = 0; i < n; i++)
    {
        const QRect rc = desktop->screenGeometry(i);
        screenRc = screenRc.united(rc);
    }

    shade = new QWidget( NULL, Qt::Window | Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );

    shade->setAttribute(Qt::WA_TranslucentBackground);
    shade->setAttribute(Qt::WA_DeleteOnClose);
    shade->setFocusPolicy( Qt::WheelFocus );
    shade->resize(screenRc.size());
    shade->move(screenRc.topLeft());
    shade->setMouseTracking( true );
    shade->installEventFilter( this );

    shade->show();
    shade->setFocus();
    shade->grabMouse(); // Beware!
}

QColor EyeDropper::GetPixel() const
{
    const QPoint pos = QCursor::pos();
    const QImage img = QPixmap::grabWindow( QApplication::desktop()->winId(), pos.x(), pos.y(), 1, 1 ).toImage();
    const QColor c = img.pixel( 0, 0 );
    return c;
}

bool EyeDropper::eventFilter( QObject* obj, QEvent* e )
{
    if ( obj == shade )
    {
        switch ( e->type() )
        {
        case QEvent::MouseMove:
            qDebug() << "MouseMove";
            //OnMouseMove();
            break;

        case QEvent::MouseButtonRelease:
            OnClicked();
            qDebug() << "MouseButtonRelease";
            return true;

        default:
            qDebug() << "Event: " << e->type();
            break;
        }
    }

    return QObject::eventFilter( obj, e );
}
