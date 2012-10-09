#include "converttexturesdialog.h"
#include "ui_converttexturesdialog.h"
#include "Qt/TextureListModel.h"
#include "Qt/TextureListDelegate.h"

#include <QComboBox>
#include <QAbstractItemModel>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"

ConvertTexturesDialog::ConvertTexturesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConvertTexturesDialog)
{
    ui->setupUi(this);
	setWindowFlags(Qt::Window);

	setupTexturesList();
	setupImagesScrollAreas();
	setupTextureListToolbar();
	setupTextureToolbar();
}

ConvertTexturesDialog::~ConvertTexturesDialog()
{
    delete ui;
}

void ConvertTexturesDialog::setupTexturesList()
{
	TextureListModel* textureListModel = new TextureListModel();
	TextureListDelegate *textureListDelegate = new TextureListDelegate();

	ui->listViewTextures->setModel(textureListModel);
	//ui->listViewTextures->setViewMode(QListView::IconMode);
	ui->listViewTextures->setItemDelegate(textureListDelegate);

	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DAVA::Scene* mainScreenScene = screen->FindCurrentBody()->bodyControl->GetScene();

	textureListModel->setScene(mainScreenScene);

	QObject::connect(ui->listViewTextures, SIGNAL(pressed(const QModelIndex &)), this, SLOT(texturePressed(const QModelIndex &)));

	textureListModel->deleteLater();
	textureListDelegate->deleteLater();
}

void ConvertTexturesDialog::setupImagesScrollAreas()
{
	QImage image("d:\\Downloads\\test.png");

	ui->scrollAreaOriginal->setImage(image);
	ui->scrollAreaCompressed->setImage(image);

	QObject::connect(ui->scrollAreaOriginal, SIGNAL(texturePosChanged(QPoint)), ui->scrollAreaCompressed, SLOT(texturePos(const QPoint &)));
	QObject::connect(ui->scrollAreaCompressed, SIGNAL(texturePosChanged(QPoint)), ui->scrollAreaOriginal, SLOT(texturePos(const QPoint &)));

	QObject::connect(ui->scrollAreaOriginal, SIGNAL(textureZoomChanged(float)), ui->scrollAreaCompressed, SLOT(textureZoom(const float &)));
	QObject::connect(ui->scrollAreaCompressed, SIGNAL(textureZoomChanged(float)), ui->scrollAreaOriginal, SLOT(textureZoom(const float &)));
}

void ConvertTexturesDialog::setupTextureToolbar()
{
	QComboBox *texturesSortCombo = new QComboBox();
	QLabel *texturesSortComboLabel = new QLabel();

	texturesSortCombo->addItem("Name");
	texturesSortCombo->addItem("Size");

	texturesSortComboLabel->setText("Sort:");

	ui->textureListToolbar->addWidget(texturesSortComboLabel);
	ui->textureListToolbar->addWidget(texturesSortCombo);
}

void ConvertTexturesDialog::setupTextureListToolbar()
{
	QComboBox *textureZoomCombo = new QComboBox();
	textureZoomCombo->addItem("10%");
	textureZoomCombo->addItem("25%");
	textureZoomCombo->addItem("50%");
	textureZoomCombo->addItem("100%");
	textureZoomCombo->addItem("150%");
	textureZoomCombo->addItem("200%");
	textureZoomCombo->addItem("400%");
	textureZoomCombo->addItem("700%");
	textureZoomCombo->addItem("1000%");

	ui->textureToolbar->addWidget(textureZoomCombo);
}

void ConvertTexturesDialog::texturePressed(const QModelIndex & index)
{
	QVariant val = index.data();
	QImage image(index.data().toString());

	ui->scrollAreaOriginal->setImage(image);
	ui->scrollAreaCompressed->setImage(image);
}
