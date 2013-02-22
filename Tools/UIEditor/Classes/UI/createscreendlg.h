#ifndef CREATESCREENDLG_H
#define CREATESCREENDLG_H

#include <QDialog>
#include "HierarchyTreeNode.h"

namespace Ui {
class CreateScreenDlg;
}

class CreateScreenDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit CreateScreenDlg(QWidget *parent = 0);
    ~CreateScreenDlg();
    
	void SetDefaultPlatform(HierarchyTreeNode::HIERARCHYTREENODEID platformId);
	QString GetScreenName() const;
	HierarchyTreeNode::HIERARCHYTREENODEID GetPlatformId() const;
	// Reimplement accept signal to avoid dialog close if planform name was not entered
	void virtual accept();
	
private:
    Ui::CreateScreenDlg *ui;
};

#endif // CREATESCREENDLG_H
