#include "ColorWidget.h"
#include "ui_ColorWidget.h"

#include <QVBoxLayout>
#include <QLabel>


ColorWidget::ColorWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ColorWidget())
{
    ui->setupUi(this);
    {
        //ui->widget->setExpanderTitle( "Test" );
    }
}

ColorWidget::~ColorWidget()
{
}
