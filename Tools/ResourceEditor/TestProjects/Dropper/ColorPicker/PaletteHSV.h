#ifndef PALETTEHSV_H
#define PALETTEHSV_H

#include "../Widgets/AbstractSlider.h"


class PaletteHSV
    : public AbstractSlider
{
    Q_OBJECT

public:
    explicit PaletteHSV(QWidget * parent);
    ~PaletteHSV();

    int GetHue() const;
    int GetSat() const;

    void setColor( int hue, int sat );
    void setColor( const QColor& c );

protected:
    QPixmap DrawBackground() const override;
    QPixmap DrawForground() const override;

private:
    void DrawCursor( QPainter *p ) const;

    QSize cursorSize;
};

#endif // PALETTEHSV_H
