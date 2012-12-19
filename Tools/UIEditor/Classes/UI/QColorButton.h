#ifndef QCOLORBUTTON_H
#define QCOLORBUTTON_H

#include <QPushButton>
#include <QColor>

class QColorButton : public QPushButton
{
    Q_OBJECT

public:
    explicit QColorButton(QWidget *parent = 0);
    // Getters/setters.
    QColor GetBackgroundColor() const;
    void SetBackgroundColor(const QColor& color);
    void SetDisplayMultipleColors(const bool needSetbackgroundImage);
signals:
    void backgroundColorChanged(QColor color);
protected:
    void SetBackgroundImage(const QString& imagePath);
private:
    QColor currentBackgroundColor;
};

#endif // QCOLORBUTTON_H
