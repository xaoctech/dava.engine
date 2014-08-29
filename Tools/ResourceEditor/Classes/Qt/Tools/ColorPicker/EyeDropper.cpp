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
#include "Platform/DpiHelper.h"


EyeDropper::EyeDropper(QWidget* parent)
    : QWidget(NULL, Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
      , mouse(new MouseHelper(this))
      , cursorSize(99, 99)
      , zoomFactor(3)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);
    
    //qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

    connect(mouse, SIGNAL( mouseMove( const QPoint& ) ), SLOT( OnMouseMove( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseRelease( const QPoint& ) ), SLOT( OnClicked( const QPoint& ) ));
    connect(mouse, SIGNAL( mouseWheel( int ) ), SLOT( OnMouseWheel( int ) ));
}

EyeDropper::~EyeDropper()
{
}

void EyeDropper::Exec()
{
    CreateShade();
    show();
    setFocus();
    update();
}

void EyeDropper::OnMouseMove(const QPoint& pos)
{
    const int sx = cursorSize.width() / 2;
    const int sy = cursorSize.height() / 2;
    QRect rcOld(QPoint(cursorPos.x() - sx, cursorPos.y() - sy), cursorSize);
    rcOld.adjust(-1, -1, 2, 2);
    QRect rcNew(QPoint(pos.x() - sx, pos.y() - sy), cursorSize);
    rcNew.adjust(-1, -1, 2, 2);
    const QRect rc = rcOld.united(rcNew);

    cursorPos = pos;
    repaint(rc);

    emit moved(GetPixel(pos));
}

void EyeDropper::OnClicked(const QPoint& pos)
{
    emit picked(GetPixel(pos));
    close();
}

void EyeDropper::OnMouseWheel(int delta)
{
    const int old = zoomFactor;

    const int max = qMin(cursorSize.width(), cursorSize.height());
    const int sign = delta > 0 ? 1 : -1;
    const double step = (zoomFactor - 1) / 2.0;

    zoomFactor += sign * qMax( int(step), 1 );
    if (zoomFactor < 1)
        zoomFactor = 1;
    if (zoomFactor > max )
        zoomFactor = max;

    if (old != zoomFactor)
    {
        update();
    }
}

void EyeDropper::paintEvent(QPaintEvent* e)
{
    Q_UNUSED( e );

    QPainter p(this);
    p.drawImage(0, 0, cache);
    DrawCursor(cursorPos, &p);
    
    p.setPen(Qt::red);
    p.drawRect(0, 0, width(), height());
}

void EyeDropper::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        close();
    }

    QWidget::keyPressEvent(e);
}

void EyeDropper::DrawCursor(const QPoint& pos, QPainter* p)
{
    const int sx = cursorSize.width() / 2 - 1;
    const int sy = cursorSize.height() / 2 - 1;
    const QColor c = GetPixel(pos);

    QRect rc(QPoint(pos.x() - sx, pos.y() - sy), QPoint(pos.x() + sx, pos.y() + sy));

    const int fc = zoomFactor;
    QRect rcZoom(QPoint(pos.x() - sx / fc, pos.y() - sy / fc), QPoint(pos.x() + sx / fc, pos.y() + sy / fc));
    const QImage& zoomed = cache.copy(rcZoom).scaled(rc.size(), Qt::KeepAspectRatio, Qt::FastTransformation);

    p->drawImage(rc, zoomed);
    p->setPen(QPen(Qt::black, 1.0));

    const int midX = (rc.left() + rc.right()) / 2;
    const int midY = (rc.bottom() + rc.top()) / 2;

    p->drawLine(rc.left(), midY, rc.right(), midY);
    p->drawLine(midX, rc.top(), midX, rc.bottom());
    p->fillRect(pos.x() - 1, pos.y() - 1, 3, 3, c);

    p->setPen(Qt::white);
    p->drawRect(rc);
    rc.adjust(-1, -1, 1, 1);
    p->setPen(Qt::black);
    p->drawRect(rc);
}

void EyeDropper::CreateShade()
{
    QDesktopWidget* desktop = QApplication::desktop();
    const int n = desktop->screenCount();
    QRect rcReal;
    QRect rcVirtual;

    for (int i = 0; i < n; i++)
    {
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
        qDebug() << "Screen: " << i << "; Real: " << rc << "; Virtual: " << screenRect;
    }
    
    qDebug() << "Done! Real: " << rcReal << "; Virtual: " << rcVirtual;
    
    const QImage img = QPixmap::grabWindow(QApplication::desktop()->winId(), rcReal.left(), rcReal.top(), rcReal.width(), rcReal.height()).toImage();
    cache = (rcReal != rcVirtual) ? img.scaled(rcVirtual.width(), rcVirtual.height()) : img;
    resize(rcVirtual.size());
    move(rcVirtual.topLeft());
    cursorPos = mapFromGlobal(QCursor::pos());
    
    const QString p = qApp->applicationDirPath();
    
    img.save(p + "/original.png", "PNG", 100);
    cache.save(p + "/scaled.png", "PNG", 100);
}

QColor EyeDropper::GetPixel(const QPoint& pos) const
{
    const QColor c = cache.pixel(pos);
    return c;
}