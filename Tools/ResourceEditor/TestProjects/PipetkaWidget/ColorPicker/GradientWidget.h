#ifndef GRADIENDWIDGET_H
#define GRADIENDWIDGET_H

#include <QWidget>


class GradientWidget
    : public QWidget
{
    Q_OBJECT

protected:
    struct Offset
    {
        int left;
        int top;
        int right;
        int bottom;

        Offset() : left(0), top(0), right(0), bottom(0) {}
    };

public:
    explicit GradientWidget(QWidget *parent);
    ~GradientWidget();

    void setColorRange( const QColor& start, const QColor& stop );
    void setDimensions( bool hor, bool ver );
    void setBgPadding( int left, int top, int right, int bottom );
    void setGrid( bool enabled, const QSize& size = QSize() );

protected:
    virtual QPixmap drawBackground() const;
    virtual QPixmap drawContent() const;

    const Offset& padding() const;

    // QWidget
    void paintEvent( QPaintEvent* e ) override;
    void resizeEvent( QResizeEvent* e ) override;

private:

    mutable QPixmap cacheBg;
    QColor startColor;
    QColor stopColor;
    Offset paddingOfs;
    bool hor;   // Direction
    bool ver;   // Direction
    bool fillBg;
    QSize gridSize;
};


#endif // GRADIENDWIDGET_H
