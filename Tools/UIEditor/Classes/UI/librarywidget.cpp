#include "Classes/UI/librarywidget.h"
#include "ui_librarywidget.h"
#include "LibraryController.h"
#include "IconHelper.h"

using namespace DAVA;

#define TEXT_ID 0

LibraryWidget::LibraryWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LibraryWidget)
{
    ui->setupUi(this);
	ui->treeWidget->clear();
	LibraryController::Instance()->Init(this);
}

LibraryWidget::~LibraryWidget()
{
    delete ui;
}

//QTreeWidgetItem* LibraryWidget::AddControl(const QString& name)
//{
//	QTreeWidgetItem* control = new QTreeWidgetItem();
//	control->setText(TEXT_ID, name);
//	control->setIcon(TEXT_ID, QIcon(IconHelper::GetIconPathForClassName(name)));
//	ui->treeWidget->addTopLevelItem(control);
//	return control;
//}

QTreeWidgetItem* LibraryWidget::AddControl(const QString& name, const QString& iconPath)
{
	QTreeWidgetItem* control = new QTreeWidgetItem();
	control->setText(TEXT_ID, name);
	control->setIcon(TEXT_ID, QIcon(iconPath));
	ui->treeWidget->addTopLevelItem(control);
	return control;
}

void LibraryWidget::RemoveControl(QTreeWidgetItem* item)
{
	int index = ui->treeWidget->indexOfTopLevelItem(item);
	if (index != -1)
	{
		delete item;
	}
}

void LibraryWidget::UpdateControl(QTreeWidgetItem* item, const QString& name)
{
	item->setText(TEXT_ID, name);
}

void LibraryWidget::SetItemVisible(QTreeWidgetItem* item, bool visible)
{
	item->setHidden(!visible);
}