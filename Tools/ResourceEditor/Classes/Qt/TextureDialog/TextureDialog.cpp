#include "TextureDialog/TextureDialog.h"
#include "TextureDialog/TextureListModel.h"
#include "TextureDialog/TextureListDelegate.h"
#include "TextureDialog/TextureConvertor.h"
#include "TextureDialog/TextureCache.h"
#include "Qt/QtUtils.h"

#include "ui_texturedialog.h"

#include <QComboBox>
#include <QAbstractItemModel>
#include <QStatusBar>
#include <QToolButton>
#include <QFileInfo>

#include "../SceneEditor/SceneEditorScreenMain.h"
#include "../SceneEditor/EditorBodyControl.h"

TextureDialog::TextureDialog(QWidget *parent)
    : QDialog(parent)
	, ui(new Ui::TextureDialog)
	, curTextureView(ViewPVR)
	, curTexture(NULL)
	, curDescriptor(NULL)
{
	// init singletones
	new TextureCache();
	new TextureConvertor();

	ui->setupUi(this);
	setWindowFlags(Qt::Window);

	textureListModel = new TextureListModel();
	textureListImagesDelegate = new TextureListDelegate();

	textureListSortModes["Name"] = TextureListModel::SortByName;
	textureListSortModes["Size"] = TextureListModel::SortBySize;

	QObject::connect(TextureConvertor::Instance(), SIGNAL(readyOriginal(const DAVA::Texture *, const QImage &)), this, SLOT(textureReadyOriginal(const DAVA::Texture *, const QImage &)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(readyPVR(const DAVA::Texture *, const DAVA::TextureDescriptor *, const QImage &)), this, SLOT(textureReadyPVR(const DAVA::Texture *, const DAVA::TextureDescriptor *, const QImage &)));
	QObject::connect(TextureConvertor::Instance(), SIGNAL(readyDXT(const DAVA::Texture *, const DAVA::TextureDescriptor *, const QImage &)), this, SLOT(textureReadyDXT(const DAVA::Texture *, const DAVA::TextureDescriptor *, const QImage &)));

	setupStatusBar();
	setupTexturesList();
	setupImagesScrollAreas();
	setupTextureListToolbar();
	setupTextureToolbar();
	setupTextureListFilter();
	setupTextureProperties();
	setupTextureViewToolbar();

	resetTextureInfo();

	// let textures list show images-view by default
	ui->actionViewImagesList->trigger();

	// let textures view show border by default
	// ui->actionShowBorder->trigger();

	// set initial empty texture
	setTexture(curTexture, curDescriptor);
	setTextureView(curTextureView);
}

TextureDialog::~TextureDialog()
{
	delete textureListImagesDelegate;
	delete textureListModel;
    delete ui;

	TextureCache::Instance()->Release();
	TextureConvertor::Instance()->Release();
}

void TextureDialog::setTexture(DAVA::Texture *texture, DAVA::TextureDescriptor *descriptor)
{
	curTexture = texture;
	curDescriptor = descriptor;

	// color channels to default value
	ui->actionColorR->setChecked(true);
	ui->actionColorG->setChecked(true);
	ui->actionColorB->setChecked(true);
	ui->actionColorA->setChecked(true);
	ui->textureAreaOriginal->setColorChannel(TextureScrollArea::ChannelAll);
	ui->textureAreaPVR->setColorChannel(TextureScrollArea::ChannelAll);

	ui->textureAreaOriginal->resetTexturePosZoom();
	ui->textureAreaPVR->resetTexturePosZoom();
	toolbarZoomSlider->setValue(0);

	// disable texture views by default
	ui->textureToolbar->setEnabled(false);
	ui->textureViewToolbar->setEnabled(false);
	ui->textureAreaOriginal->setEnabled(false);
	ui->textureAreaPVR->setEnabled(false);

	// if texture is ok - set it and enable texture views
	if(NULL != curTexture)
	{
		// enable toolbar
		ui->textureToolbar->setEnabled(true);
		ui->textureViewToolbar->setEnabled(true);

		// load original image
		// check if image is in cache
		QImage img = TextureCache::Instance()->getOriginal(curTexture);
		if(!img.isNull())
		{
			// image is in cache, so set it imidiatly (by calling image-ready slot)
			textureReadyOriginal(curTexture, img);
		}
		else
		{
			// there is no image in cache - start loading it in different thread. image-ready slot will be called 
			ui->textureAreaOriginal->setImage(QImage());
			TextureConvertor::Instance()->loadOriginal(texture);

			// show loading bar
			ui->textureAreaOriginal->waitbarVisible(true);
		}
	}
	else
	{
		// no texture - set empty images to original/PVR views
		ui->textureAreaOriginal->setImage(QImage());
		ui->textureAreaPVR->setImage(QImage());
	}

	// set info
	updateInfoOriginal();

	// set texture to properties control
	ui->textureProperties->setTexture(curTexture, curDescriptor);
}

void TextureDialog::setTextureView(TextureView view, bool forceConvert /* */)
{
	// DAVA::TextureDescriptor *curTextureDescriptor = ui->textureProperties->getTextureDescriptor();

	// if force convert - clear cached images
	if(forceConvert)
	{
		TextureCache::Instance()->clearPVR(curTexture);
		TextureCache::Instance()->clearDXT(curTexture);
	}

	curTextureView = view;

	// first set texture view to default
	ui->actionViewPVR->setChecked(false);
	ui->actionViewDXT->setChecked(false);

	// second set texture view to appropriate state
	if(NULL != curTexture && NULL != curDescriptor)
	{
		// set empty image to converted image view. it will be visible until
		// conversion done (signal by textureConvertor).
		ui->textureAreaPVR->setImage(QImage());
		ui->textureAreaPVR->waitbarVisible(true);

		switch(view)
		{
		case ViewPVR:
			{
				ui->actionViewPVR->setChecked(true);

				QImage img = TextureCache::Instance()->getPVR(curTexture);
				if(!img.isNull())
				{
					// image already in cache, call its ready slot
					textureReadyPVR(curTexture, curDescriptor, img);
				}
				else
				{
					// Start PVR convert. Signal will be emited when conversion done
					TextureConvertor::Instance()->getPVR(curTexture, curDescriptor, forceConvert);
				}
			}
			break;
		case ViewDXT:
			{
				ui->actionViewDXT->setChecked(true);

				QImage img = TextureCache::Instance()->getDXT(curTexture);
				if(!img.isNull())
				{
					// image already in cache, call its ready slot
					textureReadyDXT(curTexture, curDescriptor, img);
				}
				else
				{
					// Start DXT convert. Signal will be emited when conversion done
					TextureConvertor::Instance()->getDXT(curTexture, curDescriptor, forceConvert);
				}
			}
			break;
		default:
			break;
		}
	}

	updateInfoConverted();
}

void TextureDialog::updateInfoColor(QLabel *label, const QColor &color /* = QColor() */)
{
	if(NULL != label)
	{
		char tmp[1024];

		sprintf(tmp, "R: %02X\nG: %02X\nB: %02X\nA: %02X",
			color.red(), color.green(), color.blue(), color.alpha());

		label->setText(tmp);
	}
}

void TextureDialog::updateInfoPos(QLabel *label, const QPoint &pos /* = QPoint() */)
{
	if(NULL != label)
	{
		char tmp[1024];

		sprintf(tmp, "X : %d\nY : %d",
			pos.x(), pos.y());

		label->setText(tmp);
	}
}

void TextureDialog::updateInfoOriginal()
{
	if(NULL != curTexture && NULL != curDescriptor)
	{
		char tmp[1024];

		const char *formatStr = DAVA::Texture::GetPixelFormatString(DAVA::FORMAT_RGBA8888);

		int datasize = curTexture->width * curTexture->height * Texture::GetPixelFormatSizeInBytes(DAVA::FORMAT_RGBA8888);
		int filesize = QFileInfo(curDescriptor->GetSourceTexturePathname().c_str()).size();

		sprintf(tmp, "Format\t: %s\nSize\t: %dx%d\nData size\t: %s\nFile size\t: %s\n", formatStr, curTexture->width, curTexture->height,
			 SizeInBytesToString(datasize).c_str(),
			 SizeInBytesToString(filesize).c_str());

		ui->labelOriginalFormat->setText(tmp);
	}
	else
	{
		ui->labelOriginalFormat->setText("");
	}
}

void TextureDialog::updateInfoConverted()
{
	if(NULL != curTexture && NULL != curDescriptor)
	{
		char tmp[1024];

		const char *formatStr = "";

		int datasize = 0;
		int filesize = 0;

		switch(curTextureView)
		{
		case ViewPVR:
			if(curDescriptor->pvrCompression.format != DAVA::FORMAT_INVALID)
			{
				formatStr = DAVA::Texture::GetPixelFormatString(curDescriptor->pvrCompression.format);
				datasize = curTexture->width * curTexture->height * Texture::GetPixelFormatSizeInBytes(curDescriptor->pvrCompression.format);
				filesize = QFileInfo(Texture::GetPathnameForFileFormat(curTexture->GetPathname(), DAVA::PVR_FILE).c_str()).size();
			}
			break;
		case ViewDXT:
			if(curDescriptor->dxtCompression.format != DAVA::FORMAT_INVALID)
			{
				formatStr = DAVA::Texture::GetPixelFormatString(curDescriptor->dxtCompression.format);
				datasize = curTexture->width * curTexture->height * Texture::GetPixelFormatSizeInBytes(curDescriptor->dxtCompression.format);
				filesize = QFileInfo(Texture::GetPathnameForFileFormat(curTexture->GetPathname(), DAVA::DXT_FILE).c_str()).size();
			}
			break;
		}

		sprintf(tmp, "Format\t: %s\nSize\t: %dx%d\nData size\t: %s\nFile size\t: %s\n", formatStr, curTexture->width, curTexture->height,
			SizeInBytesToString(datasize).c_str(),
			SizeInBytesToString(filesize).c_str());

		ui->labelConvertedFormat->setText(tmp);
	}
	else
	{
		ui->labelConvertedFormat->setText("");
	}
}

void TextureDialog::setupStatusBar()
{
	statusBar = new QStatusBar(this);
	statusBarLabel = new QLabel();

	statusBar->addWidget(statusBarLabel);
	ui->mainLayout->addWidget(statusBar);

	QObject::connect(TextureConvertor::Instance(), SIGNAL(convertStatus(const JobItem *, int)), this, SLOT(convertStatus(const JobItem *, int)));
}

void TextureDialog::setupTexturesList()
{
	QObject::connect(textureListImagesDelegate, SIGNAL(needRedraw(const DAVA::Texture *)), this, SLOT(textureListItemNeedRedraw(const DAVA::Texture *)));
	textureListDefaultDelegate = ui->listViewTextures->itemDelegate();

	SceneEditorScreenMain * screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
	DAVA::Scene* mainScreenScene = screen->FindCurrentBody()->bodyControl->GetScene();

	ui->listViewTextures->setModel(textureListModel);
	textureListModel->setScene(mainScreenScene);

	QObject::connect(ui->listViewTextures, SIGNAL(selected(const QModelIndex &)), this, SLOT(texturePressed(const QModelIndex &)));
}

void TextureDialog::setupImagesScrollAreas()
{
	// pos change
	QObject::connect(ui->textureAreaOriginal, SIGNAL(texturePosChanged(const QPoint &)), ui->textureAreaPVR, SLOT(texturePos(const QPoint &)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(texturePosChanged(const QPoint &)), ui->textureAreaOriginal, SLOT(texturePos(const QPoint &)));

	// mouse over pixel
	QObject::connect(ui->textureAreaOriginal, SIGNAL(mouseOverPixel(const QPoint &)), this, SLOT(texturePixelOver(const QPoint &)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(mouseOverPixel(const QPoint &)), this, SLOT(texturePixelOver(const QPoint &)));

	// mouse wheel
	QObject::connect(ui->textureAreaOriginal, SIGNAL(mouseWheel(int)), this, SLOT(textureAreaWheel(int)));
	QObject::connect(ui->textureAreaPVR, SIGNAL(mouseWheel(int)), this, SLOT(textureAreaWheel(int)));
}

void TextureDialog::setupTextureToolbar()
{
	QLabel *textureZoomLabel = new QLabel("Zoom: ");

	toolbarZoomSlider = new QSlider();
	toolbarZoomSliderValue = new QLabel();
	toolbarZoomSlider->setOrientation(Qt::Horizontal);
	toolbarZoomSlider->setMaximumWidth(100);
	toolbarZoomSlider->setTracking(true);
	toolbarZoomSlider->setRange(-90, 90);
	toolbarZoomSlider->setTickPosition(QSlider::TicksBelow);
	toolbarZoomSlider->setTickInterval(15);
	toolbarZoomSlider->setSingleStep(5);
	textureZoomSlide(0);

	ui->textureToolbar->addWidget(textureZoomLabel);
	ui->textureToolbar->addWidget(toolbarZoomSlider);
	ui->textureToolbar->addWidget(toolbarZoomSliderValue);
	ui->textureToolbar->addSeparator();

	QObject::connect(ui->actionColorA, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorB, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorG, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));
	QObject::connect(ui->actionColorR, SIGNAL(triggered(bool)), this, SLOT(textureColorChannelPressed(bool)));

	QObject::connect(ui->actionShowBorder, SIGNAL(triggered(bool)), this, SLOT(textureBorderPressed(bool)));
	QObject::connect(ui->actionShowBgMask, SIGNAL(triggered(bool)), this, SLOT(textureBgMaskPressed(bool)));

	QObject::connect(ui->actionZoom100, SIGNAL(triggered(bool)), this, SLOT(textureZoom100(bool)));
	QObject::connect(ui->actionZoomFit, SIGNAL(triggered(bool)), this, SLOT(textureZoomFit(bool)));

	QObject::connect(toolbarZoomSlider, SIGNAL(valueChanged(int)), this, SLOT(textureZoomSlide(int)));
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

	texturesSortComboLabel->setText("Sort by: ");

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
	QObject::connect(ui->textureProperties, SIGNAL(propertyChanged()), this, SLOT(texturePropertyChanged()));
}

void TextureDialog::setupTextureViewToolbar()
{
	QObject::connect(ui->actionViewPVR, SIGNAL(triggered(bool)), this, SLOT(textureViewPVR(bool)));
	QObject::connect(ui->actionViewDXT, SIGNAL(triggered(bool)), this, SLOT(textureViewDXT(bool)));
}

void TextureDialog::resetTextureInfo()
{
	updateInfoColor(ui->labelOriginalRGBA);
	updateInfoPos(ui->labelOriginalXY);

	updateInfoColor(ui->labelConvertedRGBA);
	updateInfoPos(ui->labelConvertedXY);

	updateInfoOriginal();
	updateInfoConverted();
}

void TextureDialog::texturePressed(const QModelIndex & index)
{
	setTexture(textureListModel->getTexture(index), textureListModel->getDescriptor(index));
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

void TextureDialog::textureListItemNeedRedraw(const DAVA::Texture *texture)
{
	if(NULL != texture)
	{
		textureListModel->dataReady(texture);
	}
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

void TextureDialog::textureBorderPressed(bool checked)
{
	ui->textureAreaPVR->borderVisible(checked);
	ui->textureAreaOriginal->borderVisible(checked);
}

void TextureDialog::textureBgMaskPressed(bool checked)
{
	ui->textureAreaPVR->bgMaskVisible(checked);
	ui->textureAreaOriginal->bgMaskVisible(checked);
}

void TextureDialog::texturePropertyChanged()
{
	// set current Texture view and force texture convertion
	setTextureView(curTextureView, true);
}

void TextureDialog::textureViewPVR(bool checked)
{
	setTextureView(ViewPVR);
}

void TextureDialog::textureViewDXT(bool checked)
{
	setTextureView(ViewDXT);
}

void TextureDialog::textureReadyOriginal(const DAVA::Texture *texture, const QImage &image)
{
	if(NULL != texture)
	{
		// put this image into cache
		TextureCache::Instance()->setOriginal(texture, image);

		if(curTexture == texture)
		{
			ui->textureAreaOriginal->setImage(image);
			ui->textureAreaOriginal->setEnabled(true);
			ui->textureAreaOriginal->waitbarVisible(false);

			// set info about loaded image
			updateInfoOriginal();
		}
	}
}

void TextureDialog::textureReadyPVR(const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	// put this image into cache
	TextureCache::Instance()->setPVR(texture, image);

	if(curTexture == texture && curTextureView == ViewPVR)
	{
		ui->textureAreaPVR->setImage(image);
		ui->textureAreaPVR->setEnabled(true);
		ui->textureAreaPVR->waitbarVisible(false);

		if(NULL != texture && NULL != descriptor)
		{
			// set info about converted image
			updateInfoConverted();
		}
	}
}

void TextureDialog::textureReadyDXT(const DAVA::Texture *texture, const DAVA::TextureDescriptor *descriptor, const QImage &image)
{
	// put this image into cache
	TextureCache::Instance()->setDXT(texture, image);

	if(curTexture == texture && curTextureView == ViewDXT)
	{
		ui->textureAreaPVR->setImage(image);
		ui->textureAreaPVR->setEnabled(true);
		ui->textureAreaPVR->waitbarVisible(false);

		if(NULL != texture && NULL != descriptor)
		{
			// set info about converted image
			updateInfoConverted();
		}
	}
}

void TextureDialog::texturePixelOver(const QPoint &pos)
{
	if(ui->textureAreaOriginal->sceneRect().contains(QPointF(pos.x(), pos.y())))
	{
		QColor origColor = ui->textureAreaOriginal->getPixelColor(pos);
		QColor convColor = ui->textureAreaPVR->getPixelColor(pos);

		updateInfoPos(ui->labelOriginalXY, pos);
		updateInfoPos(ui->labelConvertedXY, pos);

		updateInfoColor(ui->labelOriginalRGBA, origColor);
		updateInfoColor(ui->labelConvertedRGBA, convColor);
	}
	else
	{
		// default pos
		updateInfoPos(ui->labelOriginalXY);
		updateInfoPos(ui->labelConvertedXY);

		// default color
		updateInfoColor(ui->labelOriginalRGBA);
		updateInfoColor(ui->labelConvertedRGBA);
	}
}

void TextureDialog::textureZoomSlide(int value)
{
	float zoom;
	int v;

	if(value > 0)
	{
		value *= 10;
	}

	zoom = 1.0 + (float) value / 100.0;
	v = 100 + value;

	ui->textureAreaOriginal->textureZoom(zoom);
	ui->textureAreaPVR->textureZoom(zoom);

	toolbarZoomSliderValue->setText(QString("%1%").arg(v));
}

void TextureDialog::textureZoomFit(bool checked)
{
	if(NULL != curTexture)
	{
		if(0 != curTexture->width && 0 != curTexture->height)
		{
			int v = 0;
			float needWidth = (float) ui->textureAreaOriginal->width();
			float needHeight = (float) ui->textureAreaOriginal->height();
			float scaleX = needWidth / (float) curTexture->width;
			float scaleY = needHeight / (float) curTexture->height;
			float scale = DAVA::Min(scaleX, scaleY);

			v = (int) ((scale - 1.0) * 100.0);

			if(v >= 0)
			{
				v = v / 10;
				v -= v % toolbarZoomSlider->singleStep();
			}
			else
			{
				v -= toolbarZoomSlider->singleStep();
				v -= v % toolbarZoomSlider->singleStep();
			}

			toolbarZoomSlider->setValue(v);
		}
	}
}

void TextureDialog::textureZoom100(bool checked)
{
	toolbarZoomSlider->setValue(0);
}

void TextureDialog::textureAreaWheel(int delta)
{
	int v = toolbarZoomSlider->value();
	v += delta / 20;
	v -= v % toolbarZoomSlider->singleStep();
	toolbarZoomSlider->setValue(v);
}

void TextureDialog::convertStatus(const JobItem *jobCur, int jobLeft)
{
	QString statusText;

	if(NULL != jobCur && NULL != jobCur->texture)
	{
		statusText += QString("Converting %1").arg(QString(jobCur->texture->GetPathname().c_str()));
	}

	if(jobLeft > 0)
	{
		statusText += QString("; %1 more texture(s) to convert").arg(QString::number(jobLeft));
	}

	statusBarLabel->setText(statusText);
}
