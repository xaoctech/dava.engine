#ifndef CREATEAGREEGATORDLG_H
#define CREATEAGREEGATORDLG_H

#include <QDialog>
#include "HierarchyTreeNode.h"

namespace Ui {
class CreateAggregatorDlg;
}

class CreateAggregatorDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit CreateAggregatorDlg(QWidget *parent = 0);
    ~CreateAggregatorDlg();
    
	void SetDefaultPlatform(HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	QString GetName() const;
	Rect GetRect() const;
	HierarchyTreeNode::HIERARCHYTREENODEID GetPlatformId() const;
	// Reimplement accept signal to avoid dialog close if planform name was not entered
	void virtual accept();
	
private:
    Ui::CreateAggregatorDlg *ui;
};

#endif //CREATEAGREEGATORDLG_H