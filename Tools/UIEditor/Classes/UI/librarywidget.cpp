#include "Classes/UI/librarywidget.h"
#include "ui_librarywidget.h"
#include "LibraryController.h"

using namespace DAVA;

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

void LibraryWidget::AddControl(const QString& name)
{
	QTreeWidgetItem* control = new QTreeWidgetItem();
	control->setText(0, name);
	ui->treeWidget->addTopLevelItem(control);
}

void LibraryWidget::RemoveControl(const QString &name)
{
	QList<QTreeWidgetItem*> items = ui->treeWidget->findItems(name, Qt::MatchExactly);
	for (int i = 0; i < items.size(); ++i)
	{
		QTreeWidgetItem* item = items[i];
		delete item;
	}
}