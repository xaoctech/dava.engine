#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

#include <QWidget>
#include <QMap>

#include "../Widgets/AbstractSlider.h"


class GradientSlider
    : public AbstractSlider
{
    Q_OBJECT

public:
    explicit GradientSlider(QWidget *parent);
    ~GradientSlider();

    void SetColors( const QColor& c1, const QColor& c2 );
    void SetDimensions( const Qt::Edges& flags );
    void SetOrientation( Qt::Orientation orientation );

protected:
    QPixmap DrawBackground() const override;
    QPixmap DrawForground() const override;

private:
    void drawArrow( Qt::Edge arrow, QPainter *p ) const;

    QColor c1;
    QColor c2;
    QSize arrowSize;
    Qt::Edges arrows;
    Qt::Orientation orientation;
    mutable QMap< Qt::Edge, QPixmap > arrowCache;
};


#endif // GRADIENTSLIDER_H
