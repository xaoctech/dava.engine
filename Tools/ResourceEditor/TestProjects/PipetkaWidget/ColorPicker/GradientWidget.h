#ifndef GRADIENDWIDGET_H
#define GRADIENDWIDGET_H

#include <QWidget>


class GradientWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit GradientWidget(QWidget *parent);
    ~GradientWidget();

    void setColorRange( const QColor& start, const QColor& stop );
    void setDimensions( bool hor, bool ver );

private:
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

    QPixmap cache;
    QColor startColor;
    QColor stopColor;
    bool hor;
    bool ver;
};


#endif // GRADIENDWIDGET_H
