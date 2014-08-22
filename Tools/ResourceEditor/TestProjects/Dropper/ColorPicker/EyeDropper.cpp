#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QFrame>

#include "../Helpers/MouseHelper.h"


EyeDropper::EyeDropper(QObject *parent)
    : QObject(parent)
{
}

EyeDropper::~EyeDropper()
{
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

    shade = new QWidget( desktop, Qt::Window | Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint );

    shade->setAttribute(Qt::WA_TranslucentBackground);
    shade->setAttribute( Qt::WA_DeleteOnClose );
    shade->resize( screenRc.size() );
    shade->move( screenRc.topLeft() );
    //shade->resize( 200, 200 );
    //shade->move( 100, 100 );
    //shade->setWindowOpacity( 0.004 );

    //shade->setBackgroundRole( QPalette::NoRole );
    //shade->setAutoFillBackground( false );
    
    //QFrame *innerFrame = new QFrame(shade);
    //QPalette framePal( shade->palette() );
    //framePal.setBrush( QPalette::Background, Qt::NoBrush );
    //framePal.setColor( QPalette::Background, Qt::transparent );
    //innerFrame->setPalette( framePal );
    ////innerFrame->setStyleSheet( "border: 1px solid red;background: transparent;" );
    //innerFrame->move( 0, 0 );
    //innerFrame->resize( shade->size() );

    //QPalette pal(shade->palette());
    //pal.setBrush( QPalette::Background, Qt::NoBrush );
    //pal.setColor( QPalette::Background, Qt::transparent );
    //shade->setPalette( pal );

    mouse = new MouseHelper( shade );
    connect( mouse, SIGNAL( clicked() ), shade, SLOT( close() ) );

    shade->show();
    shade->grabMouse();
}
