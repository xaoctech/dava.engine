#include "Gui/ButtonsWidget.h"
#include "ui_buttonswidget.h"
#include "defines.h"
#include <QPushButton>

ButtonsWidget::ButtonsWidget(int rowNum, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::ButtonsWidget)
    , rowNumber(rowNum)
{
    ui->setupUi(this);

    setRemoveEnabled(false);

    connect(this, SIGNAL(OnInstall(int)), parent, SLOT(OnInstall(int)));
    connect(this, SIGNAL(OnRun(int)), parent, SLOT(OnRun(int)));
    connect(this, SIGNAL(OnRemove(int)), parent, SLOT(OnRemove(int)));

    connect(ui->removeButton, SIGNAL(clicked()), this, SLOT(OnRemoveClicked()));
    connect(ui->runButton, SIGNAL(clicked()), this, SLOT(OnRunClicked()));
}

ButtonsWidget::~ButtonsWidget()
{
    SafeDelete(ui);
}

void ButtonsWidget::setRemoveEnabled(bool enabled)
{
    ui->removeButton->setEnabled(enabled);
}

void ButtonsWidget::OnRunClicked()
{
    emit OnRun(rowNumber);
}

void ButtonsWidget::OnRemoveClicked()
{
    emit OnRemove(rowNumber);
}
