#ifndef ABSTRACTCOLORPICKER_H
#define ABSTRACTCOLORPICKER_H

#include <QWidget>


class AbstractColorPicker
    : public QWidget
{
    Q_OBJECT

signals:
    void begin();
    void changing( const QColor& c );
    void changed( const QColor& c );
    void canceled();

public:
    explicit AbstractColorPicker( QWidget *parent );
    ~AbstractColorPicker();

    QColor GetColor() const;

protected:
    void SetColorInternal( const QColor& c );

private:
    QColor color;
};

#endif // ABSTRACTCOLORPICKER_H
