#include "Classes/UI/Dialogs/createaggregatordlg.h"
#include "ui_createaggregatordlg.h"
#include "HierarchyTreeController.h"

#include <QMessageBox>

CreateAggregatorDlg::CreateAggregatorDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateAggregatorDlg)
{
    ui->setupUi(this);
	
	const HierarchyTreeNode::HIERARCHYTREENODESLIST& platforms = HierarchyTreeController::Instance()->GetTree().GetPlatforms();
	for (HierarchyTreeNode::HIERARCHYTREENODESLIST::const_iterator iter = platforms.begin();
		 iter != platforms.end();
		 ++iter)
	{
		ui->platformsCombo->addItem((*iter)->GetName());
		ui->platformsCombo->setItemData(ui->platformsCombo->count() - 1, QVariant((*iter)->GetId()));
	}
}

CreateAggregatorDlg::~CreateAggregatorDlg()
{
    delete ui;
}

QString CreateAggregatorDlg::GetName() const
{
	return ui->lineEdit->text();
}

Rect CreateAggregatorDlg::GetRect() const
{
	return Rect(0, 0, ui->width->value(), ui->height->value());
}

HierarchyTreeNode::HIERARCHYTREENODEID CreateAggregatorDlg::GetPlatformId() const
{
	return ui->platformsCombo->itemData(ui->platformsCombo->currentIndex()).toInt();
}

void CreateAggregatorDlg::SetDefaultPlatform(HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	HierarchyTreePlatformNode* node = dynamic_cast<HierarchyTreePlatformNode*>(HierarchyTreeController::Instance()->GetTree().GetNode(platformId));
	if (!node)
	{
		node = HierarchyTreeController::Instance()->GetActivePlatform();
		if (node)
			platformId = node->GetId();
	}

	int id = ui->platformsCombo->findData(QVariant(platformId));
	ui->platformsCombo->setCurrentIndex(id);

	if (node)
	{
		ui->width->setValue(node->GetWidth());
		ui->height->setValue(node->GetHeight());
	}
}

void CreateAggregatorDlg::accept()
{
	const QString name = GetName();
	if (!name.isNull() && !name.isEmpty())
	{
		QDialog::accept();
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Please fill aggregator name field with value. It can't be empty."));
		msgBox.exec();
	}	
}