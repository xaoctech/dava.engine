#ifndef COLORVALUE_H
#define COLORVALUE_H

#include <QFrame>


class ColorValue
    : public QFrame
{
    Q_OBJECT

public:
    explicit ColorValue(QWidget *parent);
    ~ColorValue();

private:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );

    QPixmap cache;

};


#endif // COLORVALUE_H
