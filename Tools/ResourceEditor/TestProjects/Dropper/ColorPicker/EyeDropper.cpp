#include "EyeDropper.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>


EyeDropper::EyeDropper(QObject *parent)
    : QObject(parent)
{
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::CreateShades()
{
    QDesktopWidget *desktop = QApplication::desktop();
}

QWidget* EyeDropper::CreateShade( int _screen ) const
{
    QDesktopWidget *desktop = QApplication::desktop();
    QWidget *parent = desktop->screen( _screen );

    QWidget *w = new QWidget( parent, Qt::Window );
    w->setAttribute( Qt::WA_TranslucentBackground );
    w->resize( parent->size() );
    w->move( 0, 0 );
    w->show();

    return w;
}
