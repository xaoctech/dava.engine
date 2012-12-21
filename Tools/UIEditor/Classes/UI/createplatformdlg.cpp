#include "Classes/UI/createplatformdlg.h"
#include "ui_createplatformdlg.h"

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