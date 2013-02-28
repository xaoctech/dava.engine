#ifndef CREATEPLATFORMDLG_H
#define CREATEPLATFORMDLG_H

#include <QDialog>

namespace Ui {
class CreatePlatformDlg;
}

class CreatePlatformDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit CreatePlatformDlg(QWidget *parent = 0);
    ~CreatePlatformDlg();
    
	QString GetPlatformName() const;
	int GetWidth() const;
	int GetHeight() const;
	// Reimplement accept signal to avoid dialog close if planform name was not entered
	void virtual accept();

private:
    Ui::CreatePlatformDlg *ui;
};

#endif // CREATEPLATFORMDLG_H
