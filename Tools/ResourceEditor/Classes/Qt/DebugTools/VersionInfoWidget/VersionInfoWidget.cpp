#include "VersionInfoWidget.h"

#include "ui_VersionInfoWidget.h"

VersionInfoWidget::VersionInfoWidget(QWidget* parent)
    : QWidget(parent)
    , ui( new Ui::VersionInfoWidget() )
{
    ui->setupUi(this);
}

VersionInfoWidget::~VersionInfoWidget()
{
}
