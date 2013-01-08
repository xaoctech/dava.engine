#include "Classes/UI/createplatformdlg.h"
#include "ui_createplatformdlg.h"
#include <QMessageBox>

CreatePlatformDlg::CreatePlatformDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatePlatformDlg)
{
    ui->setupUi(this);
}

CreatePlatformDlg::~CreatePlatformDlg()
{
    delete ui;
}

QString CreatePlatformDlg::GetPlatformName() const
{
	return ui->lineEdit->text();
}

int CreatePlatformDlg::GetWidth() const
{
	return ui->width->value();
}

int CreatePlatformDlg::GetHeight() const
{
	return ui->height->value();
}

void CreatePlatformDlg::accept()
{
	const QString platformName = GetPlatformName();
	if (!platformName.isNull() && !platformName.isEmpty())
	{
		QDialog::accept();
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Please fill platform name field with value. It can't be empty."));
		msgBox.exec();
	}	
}