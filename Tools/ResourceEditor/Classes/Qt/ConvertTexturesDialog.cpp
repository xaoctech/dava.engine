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

	textureListModel = new TextureListModel();
	textureListImagesDelegate = new TextureListDelegate();

    ui->setupUi(this);
	setWindowFlags(Qt::Window);

	setupTexturesList();
	setupImagesScrollAreas();
	setupTextureListToolbar();
	setupTextureToolbar();
	setupTextureListFilter();

	ui->actionViewImagesList->trigger();
}

ConvertTexturesDialog::~ConvertTexturesDialog()
{
	delete textureListImagesDelegate;
	delete textureListModel;
    delete ui;
}

void ConvertTexturesDialog::setupTexturesList()
{
	textureListDefaultDelegate = ui->listViewTextures->itemDelegate();

	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DAVA::Scene* mainScreenScene = screen->FindCurrentBody()->bodyControl->GetScene();

	ui->listViewTextures->setModel(textureListModel);
	textureListModel->setScene(mainScreenScene);

	QObject::connect(ui->listViewTextures, SIGNAL(pressed(const QModelIndex &)), this, SLOT(texturePressed(const QModelIndex &)));
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

	QObject::connect(ui->actionViewTextList, SIGNAL(triggered(bool)), this, SLOT(textureListViewText(bool)));
	QObject::connect(ui->actionViewImagesList, SIGNAL(triggered(bool)), this, SLOT(textureListViewImages(bool)));
}

void ConvertTexturesDialog::setupTextureListFilter()
{
	QObject::connect(ui->textureFilterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textureListFilterChanged(const QString&)));
}

void ConvertTexturesDialog::texturePressed(const QModelIndex & index)
{
	QVariant val = index.data();
	QImage image(index.data().toString());

	ui->scrollAreaOriginal->setImage(image);
	ui->scrollAreaCompressed->setImage(image);
}

void ConvertTexturesDialog::textureListViewText(bool checked)
{
	ui->actionViewImagesList->setChecked(false);
	ui->actionViewTextList->setChecked(true);

	ui->listViewTextures->setItemDelegate(textureListDefaultDelegate);
	ui->listViewTextures->reset();
}

void ConvertTexturesDialog::textureListViewImages(bool checked)
{
	ui->actionViewTextList->setChecked(false);
	ui->actionViewImagesList->setChecked(true);

	ui->listViewTextures->setItemDelegate(textureListImagesDelegate);
	ui->listViewTextures->reset();
}

void ConvertTexturesDialog::textureListFilterChanged(const QString &text)
{
	textureListModel->setFilter(text);
}