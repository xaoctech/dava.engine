#ifndef ABSTRACTCOLORPALETTE_H
#define ABSTRACTCOLORPALETTE_H

#include <QWidget>


class AbstractColorPalette
    : public QWidget
{
    Q_OBJECT

signals:
    void begin();
    void changing( const QColor& c );
    void changed( const QColor& c );
    void canceled();

public:
    explicit AbstractColorPalette(QWidget *parent);
    ~AbstractColorPalette();

    virtual QColor GetColor() const = 0;
    virtual void setColor( const QColor& c ) = 0;

private:
};


#endif // ABSTRACTCOLORPALETTE_H
