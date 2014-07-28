#ifndef COLORPALETTE_H
#define COLORPALETTE_H

#include <QFrame>


class ColorPalette
    : public QFrame
{
    Q_OBJECT

public:
    explicit ColorPalette(QWidget *parent);
    ~ColorPalette();

private:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );

    QPixmap cache;

private:
    static int hue(const QPoint& pt, const QSize& size);
    static int sat( const QPoint& pt, const QSize& size );
    static int val( const QPoint& pt, const QSize& size );
};

#endif // COLORPALETTE_H
