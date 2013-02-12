#include "Classes/UI/createscreendlg.h"
#include "ui_createscreendlg.h"
#include "HierarchyTreeController.h"

CreateScreenDlg::CreateScreenDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateScreenDlg)
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

CreateScreenDlg::~CreateScreenDlg()
{
    delete ui;
}

QString CreateScreenDlg::GetScreenName() const
{
	return ui->lineEdit->text();
}

HierarchyTreeNode::HIERARCHYTREENODEID CreateScreenDlg::GetPlatformId() const
{
	return ui->platformsCombo->itemData(ui->platformsCombo->currentIndex()).toInt();
}

void CreateScreenDlg::SetDefaultPlatform(HierarchyTreeNode::HIERARCHYTREENODEID platformId)
{
	int id = ui->platformsCombo->findData(QVariant(platformId));
	if (id < 0) id = 0;
	ui->platformsCombo->setCurrentIndex(id);
}