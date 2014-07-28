#include "ColorWidget.h"
#include "ui_ColorWidget.h"

ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ColorWidget())
{
    ui->setupUi(this);
}

ColorWidget::~ColorWidget()
{
}
