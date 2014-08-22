#include "dropper.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QCursor>
#include <QDateTime>
#include <QDebug>


#include "ColorPicker/ColorPicker.h"
#include "ColorPicker/EyeDropper.h"


Dropper::Dropper(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    ui.test_1->SetColors( QColor( 255, 255, 0 ), QColor( 255, 0, 0 ) );
    ui.test_1->SetDimensions( Qt::BottomEdge );
    ui.test_1->SetOrientation( Qt::Horizontal );

    ui.test_2->SetColors( QColor( 255, 255, 128, 10 ), QColor( 0, 255, 128, 10 ) );
    ui.test_2->SetDimensions( Qt::LeftEdge | Qt::RightEdge );
    ui.test_2->SetOrientation( Qt::Vertical );
    ui.test_2->SetOffsets( 5, 20, 5, 0 );

    ui.test_3->SetColor( Qt::yellow );

    ui.test_4->SetColorOld( QColor( 255, 0, 255, 100 ) );
    ui.test_4->SetColorNew( QColor( 255, 255, 100, 50 ) );

    connect( ui.btn, SIGNAL( clicked() ), SLOT( showCP() ) );
    connect( ui.pick, SIGNAL( clicked() ), SLOT( testGrab2() ) );
}

Dropper::~Dropper()
{
}

void Dropper::showCP()
{
    ColorPicker *cp = new ColorPicker( this );
    cp->setWindowFlags( Qt::Window );
    cp->setAttribute( Qt::WA_DeleteOnClose, true );

    cp->SetColor( QColor( 90, 150, 230, 120 ) );
    cp->exec();
}

void Dropper::testGrab()
{
    const int n = QApplication::desktop()->screenCount();
    QRect screenRc;

    for ( int i = 0; i < n; i++ )
    {
        const QRect rc = QApplication::desktop()->screenGeometry( i );
        screenRc = screenRc.united( rc );
        qDebug() << rc;
    }

    const QPixmap pix = QPixmap::grabWindow( QApplication::desktop()->winId(), screenRc.left(), screenRc.top(), screenRc.width(), screenRc.height() );
    const QDateTime now = QDateTime::currentDateTime();
    const QPoint pos = QCursor::pos();
    const QString path = QString( "%1/test_%2-%3_%4x%5.png" )
        .arg( QApplication::applicationDirPath() )
        .arg( now.time().hour() ).arg( now.time().minute() )
        .arg( pos.x() ).arg( pos.y() );

    pix.save( path, "PNG", 100 );
}

void Dropper::testGrab2()
{
    EyeDropper *dropper = new EyeDropper( this );
    dropper->Exec();
}
