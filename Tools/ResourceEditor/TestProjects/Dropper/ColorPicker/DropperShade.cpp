#include "DropperShade.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>



DropperShade::DropperShade(QWidget *parent)
    : QWidget(parent, Qt::Popup | Qt::CustomizeWindowHint)
{
    //setAttribute( Qt::WA_TranslucentBackground );
    setAttribute( Qt::WA_DeleteOnClose );
}

DropperShade::~DropperShade()
{
}
