#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

#include <QWidget>
#include <QMap>


#include "GradientWidget.h"
#include "IColorEditor.h"
#include "MouseHelper.h"


class GradientSlider
    : public GradientWidget
    , public IColorEditor
{
    Q_OBJECT

signals:
    // IColorEditor overrides
    void begin();
    void changing( QColor const& c );
    void changed( QColor const& c );
    void canceled();

public:
    explicit GradientSlider(QWidget *parent);
    ~GradientSlider();

    void setEditorDimensions( Qt::Edges flags = (Qt::LeftEdge | Qt::RightEdge) );

    // IColorEditor overrides
    QColor GetColor() const override;
    void setColor( QColor const& c ) override;

protected:
    // GradientWidget overrides
    QPixmap drawContent() const override;

private slots:
    void onMousePress( const QPoint& pos );
    void onMouseMove( const QPoint& pos );
    void onMouseRelease( const QPoint& pos );

private:
    QPoint fitInGradient( const QPoint& pos ) const;
    void setPos( const QPoint& pos );

    void drawArrows( QPainter *p ) const;
    void drawArrow( Qt::Edge arrow, QPainter *p ) const;

    QPointer<MouseHelper> mouse;
    QSize arrowSize;
    Qt::Edges arrows;
    QPoint currentPos;
    QColor startColor;
    QColor color;

    QMap< Qt::Edge, QPixmap > arrowCache;

private:
};

#endif // GRADIENTSLIDER_H
