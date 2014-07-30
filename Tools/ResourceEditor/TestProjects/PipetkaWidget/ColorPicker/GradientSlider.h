#ifndef GRADIENTSLIDER_H
#define GRADIENTSLIDER_H

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
    void onMousePress();
    void onMouseRelease();

private:
    QPointer<MouseHelper> mouse;
    QSize arrowSize;
    Qt::Edges arrows;
    QPoint currentPos;
};

#endif // GRADIENTSLIDER_H
