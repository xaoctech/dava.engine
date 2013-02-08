#include "ModificationWidget.h"
#include "ui_ModificationWidget.h"

#include <QKeyEvent>

ModificationWidget::ModificationWidget(QWidget* parent)
:	QWidget(parent),
ui(new Ui::ModificationWidget)
{
	ui->setupUi(this);

	connect(ui->xAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(ui->yAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
	connect(ui->zAxisModify, SIGNAL(editingFinished()), this, SLOT(OnEditingFinished()));
}

ModificationWidget::~ModificationWidget()
{
	delete ui;
}

void ModificationWidget::OnEditingFinished()
{
	double x = ui->xAxisModify->value();
	double y = ui->yAxisModify->value();
	double z = ui->zAxisModify->value();

	if (x != 0.f || y != 0.f || z != 0.f)
	{
		emit ApplyModification(x, y, z);
		ResetSpinBoxes();
	}
}

void ModificationWidget::ResetSpinBoxes()
{
	this->blockSignals(true);
	ui->xAxisModify->setValue(0.0);
	ui->yAxisModify->setValue(0.0);
	ui->zAxisModify->setValue(0.0);
	this->blockSignals(false);
}

void ModificationWidget::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape)
	{
		ResetSpinBoxes();
	}
}
