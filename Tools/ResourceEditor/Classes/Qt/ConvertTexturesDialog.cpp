#include "converttexturesdialog.h"
#include "ui_converttexturesdialog.h"
#include "Qt/TextureListModel.h"

ConvertTexturesDialog::ConvertTexturesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConvertTexturesDialog)
{
    ui->setupUi(this);
	
	TextureListModel* textureListModel = new TextureListModel();
	TextureListDelegate *textureListDelegate = new TextureListDelegate();

	ui->listViewTextures->setModel(textureListModel);
	ui->listViewTextures->setItemDelegate(textureListDelegate);

	textureListModel->deleteLater();
	textureListDelegate->deleteLater();
}

ConvertTexturesDialog::~ConvertTexturesDialog()
{
    delete ui;
}
