#include "TextureDialog/TextureDialog.h"
#include "TextureDialog/TextureListModel.h"
#include "TextureDialog/TextureListDelegate.h"

#include "ui_texturedialog.h"

#include <QComboBox>
#include <QAbstractItemModel>
#include <QStatusBar>
#include <QToolButton>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"

TextureDialog::TextureDialog(QWidget *parent)
    : QDialog(parent)
	, ui(new Ui::TextureDialog)
	, curTextureOriginal(NULL)
	, curTextureView(ViewPVR)
{
	textureListModel = new TextureListModel();
	textureListImagesDelegate = new TextureListDelegate();

	textureListSortModes["Name"] = TextureListModel::SortByName;
	textureListSortModes["Size"] = TextureListModel::SortBySize;

    ui->setupUi(this);
	setWindowFlags(Qt::Window);

	setupStatusBar();
	setupTexturesList();
	setupImagesScrollAreas();
	setupTextureListToolbar();
	setupTextureToolbar();
	setupTextureListFilter();
	setupTextureProperties();
	setupTextureViewToolbar();

	setTextureView(curTextureView);

	// set textures list to show images
	ui->actionViewImagesList->trigger();

	// debug
	/*
	QImage image("d:\\Downloads\\test.png");
	ui->textureAreaOriginal->setImage(image);
	ui->textureAreaPVR->setImage(image);
	*/
}

TextureDialog::~TextureDialog()
{
	delete textureListImagesDelegate;
	delete textureListModel;
    delete ui;
}

void TextureDialog::setTextureView(TextureView view)
{
	curTextureView = view;

	// color channels to default value
	ui->actionColorR->setChecked(true);
	ui->actionColorG->setChecked(true);
	ui->actionColorB->setChecked(true);
	ui->actionColorA->setChecked(true);
	ui->textureAreaOriginal->setColorChannel(TextureScrollArea::ChannelAll);
	ui->textureAreaPVR->setColorChannel(TextureScrollArea::ChannelAll);
	ui->textureToolbar->setEnabled(false);
	ui->textureViewToolbar->setEnabled(false);

	// texture view - 1. to default
	ui->actionViewPVR->setChecked(false);
	ui->actionViewDXT->setChecked(false);

	// texture view - 2. to appropriate state
	if(NULL != curTextureOriginal)
	{
		ui->textureToolbar->setEnabled(true);
		ui->textureViewToolbar->setEnabled(true);

		QImage image(curTextureOriginal->GetPathname().c_str());
		ui->textureAreaOriginal->setImage(image);

		switch(view)
		{
		case ViewPVR:
			{
				ui->actionViewPVR->setChecked(true);
			}
			break;
		case ViewDXT:
			{
				ui->actionViewDXT->setChecked(true);
			}
			break;
		default:
			break;
		}
	}
	else
	{
		// no texture - set empty images to original/PVR views
		ui->textureAreaOriginal->setImage(QImage());
		ui->textureAreaPVR->setImage(QImage());
	}

	ui->textureProperties->setTexture(curTextureOriginal);
}

void TextureDialog::setupStatusBar()
{
	statusBar = new QStatusBar(this);
	ui->mainLayout->addWidget(statusBar);
}

void TextureDialog::setupTexturesList()
{
	textureListDefaultDelegate = ui->listViewTextures->itemDelegate();

	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DAVA::Scene* mainScreenScene = screen->FindCurrentBody()->bodyControl->GetScene();

	ui->listViewTextures->setModel(textureListModel);
	textureListModel->setScene(mainScreenScene);

	QObject::connect(ui->listViewTextures, SIGNAL(selected(const QModelIndex &)), this, SLOT(texturePressed(const QModelIndex &)));
}

void TextureDialog::setupImagesScrollAreas()
{
	QObject::connect(ui->textureAreaOriginal, SIGNAL(texturePosChanged(QPoint)), ui->textureAreaPVR, SLOT(texturePos(const QPoint &)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(texturePosChanged(QPoint)), ui->textureAreaOriginal, SLOT(texturePos(const QPoint &)));

	QObject::connect(ui->textureAreaOriginal, SIGNAL(textureZoomChanged(float)), ui->textureAreaPVR, SLOT(textureZoom(const float &)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(textureZoomChanged(float)), ui->textureAreaOriginal, SLOT(textureZoom(const float &)));
}

void TextureDialog::setupTextureToolbar()
{
	/*
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
	*/

	QObject::connect(ui->actionColorA, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorB, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorG, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorR, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
}

void TextureDialog::setupTextureListToolbar()
{
	QComboBox *texturesSortCombo = new QComboBox();
	QLabel *texturesSortComboLabel = new QLabel();

	QMapIterator<QString, int> i(textureListSortModes);
	while(i.hasNext())
	{
		i.next();
		texturesSortCombo->addItem(i.key());
	}

	texturesSortComboLabel->setText("Sort: ");

	ui->textureListToolbar->addWidget(texturesSortComboLabel);
	ui->textureListToolbar->addWidget(texturesSortCombo);

	QObject::connect(texturesSortCombo, SIGNAL(activated(const QString &)), this, SLOT(textureListSortChanged(const QString &)));
	QObject::connect(ui->actionViewTextList, SIGNAL(triggered(bool)), this, SLOT(textureListViewText(bool)));
	QObject::connect(ui->actionViewImagesList, SIGNAL(triggered(bool)), this, SLOT(textureListViewImages(bool)));
}

void TextureDialog::setupTextureListFilter()
{
	QObject::connect(ui->textureFilterEdit, SIGNAL(textChanged(const QString &)), this, SLOT(textureListFilterChanged(const QString&)));
}

void TextureDialog::setupTextureProperties()
{
	QObject::connect(ui->textureProperties, SIGNAL(formatChangedPVR(const DAVA::PixelFormat &)), this, SLOT(textureFormatPVRChanged(const DAVA::PixelFormat &)));
	QObject::connect(ui->textureProperties, SIGNAL(formatChangedDXT(const DAVA::PixelFormat &)), this, SLOT(textureFormatDXTChanged(const DAVA::PixelFormat &)));
}

void TextureDialog::setupTextureViewToolbar()
{
	QObject::connect(ui->actionViewPVR, SIGNAL(triggered(bool)), this, SLOT(textureViewPVR(bool)));
	QObject::connect(ui->actionViewDXT, SIGNAL(triggered(bool)), this, SLOT(textureViewDXT(bool)));
}

void TextureDialog::texturePressed(const QModelIndex & index)
{
	curTextureOriginal = textureListModel->texture(index);
	setTextureView(curTextureView);
}

void TextureDialog::textureListViewText(bool checked)
{
	ui->actionViewImagesList->setChecked(false);
	ui->actionViewTextList->setChecked(true);

	ui->listViewTextures->setItemDelegate(textureListDefaultDelegate);
	ui->listViewTextures->reset();
}

void TextureDialog::textureListViewImages(bool checked)
{
	ui->actionViewTextList->setChecked(false);
	ui->actionViewImagesList->setChecked(true);

	ui->listViewTextures->setItemDelegate(textureListImagesDelegate);
	ui->listViewTextures->reset();
}

void TextureDialog::textureListFilterChanged(const QString &text)
{
	textureListModel->setFilter(text);
}

void TextureDialog::textureListSortChanged(const QString &text)
{
	textureListModel->setSortMode((TextureListModel::TextureListSortMode) textureListSortModes[text]);
}

void TextureDialog::textureColorChannelPressed(bool checked)
{
	int channelsMask = 0;

	if(ui->actionColorA->isChecked()) channelsMask |= TextureScrollArea::ChannelA;
	if(ui->actionColorB->isChecked()) channelsMask |= TextureScrollArea::ChannelB;
	if(ui->actionColorG->isChecked()) channelsMask |= TextureScrollArea::ChannelG;
	if(ui->actionColorR->isChecked()) channelsMask |= TextureScrollArea::ChannelR;

	ui->textureAreaOriginal->setColorChannel(channelsMask);
	ui->textureAreaPVR->setColorChannel(channelsMask);
}

void TextureDialog::textureFormatPVRChanged(const DAVA::PixelFormat &newFormat)
{
	//ui->tabFormats->setTabEnabled(0, DAVA::FORMAT_INVALID != newFormat);
}

void TextureDialog::textureFormatDXTChanged(const DAVA::PixelFormat &newFormat)
{
	//ui->tabFormats->setTabEnabled(1, DAVA::FORMAT_INVALID != newFormat);
}

void TextureDialog::textureViewPVR(bool checked)
{
	setTextureView(ViewPVR);
}

void TextureDialog::textureViewDXT(bool checked)
{
	setTextureView(ViewDXT);
}
