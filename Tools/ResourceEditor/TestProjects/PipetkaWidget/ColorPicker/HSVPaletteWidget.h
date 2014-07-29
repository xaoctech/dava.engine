#ifndef HSVPALETTEWIDGET_H
#define HSVPALETTEWIDGET_H

#include "AbstractColorPalette.h"

class HSVPaletteWidget
    : public AbstractColorPalette
{
private:
    Q_OBJECT

public:
    explicit HSVPaletteWidget(QWidget *parent = NULL);
    ~HSVPaletteWidget();

    // AbstractColorPalette
    QColor GetColor() const;
    void setColor( QColor const& c );

private:
    void paintEvent( QPaintEvent* e );
    void resizeEvent( QResizeEvent* e );

    QColor color;
    QPixmap cache;
};

#endif // HSVPALETTEWIDGET_H
