#ifndef DROPPERSHADE_H
#define DROPPERSHADE_H

#include <QWidget>


class MouseHelper;

class DropperShade
    : public QWidget
{
    Q_OBJECT

signals:
    void clicked();
    void hovered();

public:
    explicit DropperShade(QWidget *parent);
    ~DropperShade();

private:
};


#endif // DROPPERSHADE_H
