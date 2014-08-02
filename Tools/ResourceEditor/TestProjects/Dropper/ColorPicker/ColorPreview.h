#ifndef COLORPREVIEW_H
#define COLORPREVIEW_H

#include <QWidget>

class ColorPreview : public QWidget
{
    Q_OBJECT

public:
    explicit ColorPreview(QWidget *parent);
    ~ColorPreview();

    void setColorOld( const QColor& c );
    void setColorNew( const QColor& c );

private:
    void paintEvent( QPaintEvent* e ) override;

    QColor cOld;
    QColor cNew;
    QBrush bgBrush;
};

#endif // COLORPREVIEW_H
