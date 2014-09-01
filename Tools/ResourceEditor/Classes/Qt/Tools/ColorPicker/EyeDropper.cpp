#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QDebug>

#include <QTimer>


#include "../Helpers/MouseHelper.h"
#include "DropperShade.h"
#include "Platform/DpiHelper.h"


EyeDropper::EyeDropper(QWidget* parent)
    : QObject(parent)
    , parentWidget(parent)
{
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::Exec()
{
    InitShades();
}

void EyeDropper::OnDone()
{
    for ( int i = 0; i < shades.size(); i++ )
    {
        DropperShade *shade = shades[i];
        if (shade)
        {
            shades[i]->deleteLater();
        }
    }
}

void EyeDropper::InitShades()
{
    QDesktopWidget* desktop = QApplication::desktop();
    const int n = desktop->screenCount();
    QRect rcReal;
    QRect rcVirtual;

    shades.resize(n);

    for (int i = 0; i < n; i++)
    {
        QWidget *s = desktop->screen(i);
        const QRect screenRect = desktop->screenGeometry(i);
        const double scale = DAVA::DPIHelper::GetDpiScaleFactor(i);
        rcVirtual = rcVirtual.unite(screenRect);
        
        QRect rc = screenRect;
        if (scale > 1.0)
        {
            rc.setWidth( int(scale * screenRect.width()) );
            rc.setHeight( int(scale * screenRect.height()) );
        }
        
        rcReal = rcReal.united(rc);

        const QImage img = QPixmap::grabWindow( s->winId(), rc.left(), rc.top(), rc.width(), rc.height() ).toImage();
        DropperShade *shade = new DropperShade( img, screenRect );
        shades[i] = shade;
        shade->show();

        connect( shade, SIGNAL( canceled() ), SIGNAL( canceled() ) );
        connect( shade, SIGNAL( picked(const QColor&) ), SIGNAL( picked(const QColor&) ) );
        connect( shade, SIGNAL( moved(const QColor&) ), SIGNAL( moved(const QColor&) ) );

        connect( shade, SIGNAL( canceled() ), SLOT( OnDone() ) );
        connect( shade, SIGNAL( picked(const QColor&) ), SLOT( OnDone() ) );

        //const QString path = QString( "%1/%2_%3.png" ).arg( QApplication::applicationDirPath() ).arg( "shade" ).arg( i );
        //img.save( path, "PNG", 100 );
    }
    
    //const QImage img = QPixmap::grabWindow(QApplication::desktop()->winId(), rcReal.left(), rcReal.top(), rcReal.width(), rcReal.height()).toImage();
}
