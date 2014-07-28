#ifndef COLORWIDGET_H
#define COLORWIDGET_H

#include <QWidget>
#include <QScopedPointer>

namespace Ui {class ColorWidget;};

class ColorWidget
    : public QWidget
{
    Q_OBJECT

public:
    explicit ColorWidget(QWidget *parent = 0);
    ~ColorWidget();

private:
    QScopedPointer<Ui::ColorWidget> ui;
};

#endif // COLORWIDGET_H
