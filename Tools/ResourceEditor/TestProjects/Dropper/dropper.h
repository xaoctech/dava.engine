#ifndef DROPPER_H
#define DROPPER_H

#include <QtWidgets/QWidget>
#include "ui_dropper.h"

class Dropper : public QWidget
{
    Q_OBJECT

public:
    explicit Dropper(QWidget *parent = 0);
    ~Dropper();

private slots:
    void showCP();
    void testGrab();
    void testGrab2();

private:
    Ui::DropperClass ui;
};

#endif // DROPPER_H
