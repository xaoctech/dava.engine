#include "LabeledSlider.h"
#include "ui_LabeledSlider.h"

#include "DAVAEngine.h"

#include <QSlider>
#include <QLabel>

LabeledSlider::LabeledSlider(QWidget *parent /*= 0*/)
	: QWidget(parent)
    , ui(new Ui::LabeledSlider)
{
	ui->setupUi(this);

    connect(ui->slider, SIGNAL(valueChanged(int)), SLOT(ValueChanged(int)));
    connect(ui->slider, SIGNAL(rangeChanged(int, int)), SLOT(RangeChanged(int, int)));
    
    ui->slider->setRange(0, 1);
    ui->slider->setValue(1);
}


LabeledSlider::~LabeledSlider()
{

}


int LabeledSlider::value() const
{
    return ui->slider->value();
}

int LabeledSlider::minimum() const
{
    return ui->slider->minimum();
}

void LabeledSlider::setMinimum(int min)
{
    ui->slider->setMinimum(min);
}

int LabeledSlider::maximum() const
{
    return ui->slider->maximum();
}

void LabeledSlider::setMaximum(int max)
{
    ui->slider->setMaximum(max);
}

void LabeledSlider::setRange(int min, int max)
{
    ui->slider->setRange(min, max);
}

void LabeledSlider::setValue(int val)
{
    ui->slider->setValue(val);
}

void LabeledSlider::ValueChanged(int value)
{
    ui->valueText->setText(QString::fromAscii(DAVA::Format("%d", value)));
    
    emit valueChanged(value);
}

void LabeledSlider::RangeChanged(int min, int max)
{
    ui->minText->setText(QString::fromAscii(DAVA::Format("%d", min)));
    ui->maxText->setText(QString::fromAscii(DAVA::Format("%d", max)));
}
