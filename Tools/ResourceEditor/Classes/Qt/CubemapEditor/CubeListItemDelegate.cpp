#include "CubemapEditor/CubeListItemDelegate.h"
#include "CubemapEditor/CubemapUtils.h"
#include "Render/Texture.h"
#include "Render/TextureDescriptor.h"

const int FACE_IMAGE_SIZE = 64;
const int FACE_IMAGE_BORDER = 5;
const int LIST_ITEM_OFFSET = 1;
const int LIST_ITEMS_PER_PAGE = 9;

const QColor SELECTION_BORDER_COLOR = QColor(0, 0, 0, 50);
const int SELECTION_COLOR_ALPHA = 100;

const QString FONT_NAMES[] = {QString("Arial"), QString("Arial"), QString("Arial")};
const int FONT_SIZES[] = {16, 11, 11};
const int FONT_STYLES[] = {QFont::Bold, QFont::Normal, QFont::Normal};
const int FONT_ATTRIBUTE_COUNT = 3;

const int TITLE_ITEM_DATA = 0;
const int DESCRIPTION_ITEM_DATA = 1;
const int FACE_SIZE_ITEM_DATA = 2;

CubeListItemDelegate::CubeListItemDelegate(QObject *parent)
{	
	int textTotalHeight = 0;
	for(int i = 0; i < FONT_ATTRIBUTE_COUNT; ++i)
	{
		QFontMetrics fontMetrics = QFontMetrics(QFont(FONT_NAMES[i], FONT_SIZES[i], FONT_STYLES[i]));
		textTotalHeight += GetAdjustedTextHeight(fontMetrics.height());
	}
	
	itemHeight = std::max(textTotalHeight, FACE_IMAGE_SIZE + 2 * FACE_IMAGE_BORDER + 2 * LIST_ITEM_OFFSET + 1);
}

CubeListItemDelegate::~CubeListItemDelegate()
{
	ClearCache();
}

int CubeListItemDelegate::GetAdjustedTextHeight(int baseHeight) const
{
	return baseHeight + FACE_IMAGE_BORDER;
}

void CubeListItemDelegate::ClearCache()
{
	for(std::map<std::string, QImage*>::iterator i = iconsCache.begin();
		i != iconsCache.end();
		++i)
	{
		delete i->second;
	}
	
	iconsCache.clear();
	iconSizeCache.clear();
}

void CubeListItemDelegate::UpdateCache(QStringList& filesList)
{
	ClearCache();
	
	for(int i = 0; i < filesList.size(); ++i)
	{
		QString str = filesList.at(i);
		
		DAVA::Vector<DAVA::String> faceNames;
		CubemapUtils::GenerateFaceNames(str.toStdString(), faceNames);
		for(int faceIndex = 0; faceIndex < faceNames.size(); ++faceIndex)
		{
			QImage* scaledFace = NULL;
			QImage faceImage;
			
			faceImage.load(faceNames[faceIndex].c_str());
			
			QImage scaledFaceTemp = faceImage.scaled(FACE_IMAGE_SIZE, FACE_IMAGE_SIZE);
			scaledFace = new QImage(scaledFaceTemp);
			
			iconsCache[faceNames[faceIndex]] = scaledFace;
			iconSizeCache[faceNames[faceIndex]] = QSize(faceImage.width(), faceImage.height());
		}
	}
}

void CubeListItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	int y = option.rect.y();
	int totalImageStripWidth = (FACE_IMAGE_SIZE + FACE_IMAGE_BORDER) * CubemapUtils::GetMaxFaces();
	int faceStartX = option.rect.width() - totalImageStripWidth - FACE_IMAGE_BORDER;

	QRect r = option.rect;

	QPen fontPen(QColor::fromRgb(51, 51, 51), 1, Qt::SolidLine);
	
	//BACKGROUND ALTERNATING COLORS
	painter->setBrush((index.row() % 2) ? Qt::white : QColor(250, 250, 250));
	painter->drawRect(r);
	
	painter->setPen(fontPen);
	
	QString displayText = index.data(Qt::DisplayRole).toString();
	QString title = index.data(CUBELIST_DELEGATE_ITEMFILENAME).toString();
	QString description = index.data(CUBELIST_DELEGATE_ITEMFULLPATH).toString();
	
	int textStartY = y;
	//TITLE
	painter->setFont(QFont(FONT_NAMES[TITLE_ITEM_DATA], FONT_SIZES[TITLE_ITEM_DATA], FONT_STYLES[TITLE_ITEM_DATA]));
	QFontMetrics fontMetrics = painter->fontMetrics();
	QRect textRect = fontMetrics.boundingRect(title);
	painter->drawText(FACE_IMAGE_BORDER, textStartY + GetAdjustedTextHeight(textRect.height()), title);
	textStartY += GetAdjustedTextHeight(textRect.height());
	
	//DESCRIPTION
	painter->setFont(QFont(FONT_NAMES[DESCRIPTION_ITEM_DATA], FONT_SIZES[DESCRIPTION_ITEM_DATA], FONT_STYLES[DESCRIPTION_ITEM_DATA]));
	fontMetrics = painter->fontMetrics();
	textRect = fontMetrics.boundingRect(description);
	painter->drawText(FACE_IMAGE_BORDER, textStartY + GetAdjustedTextHeight(textRect.height()), description);
	textStartY += GetAdjustedTextHeight(textRect.height());
	
	int imageWidth = 0;
	int imageHeight = 0;
	DAVA::Vector<DAVA::String> faceNames;
	CubemapUtils::GenerateFaceNames(description.toStdString(), faceNames);
	for(int i = 0; i < faceNames.size(); ++i)
	{
		std::map<std::string, QImage*>::const_iterator cachedImage = iconsCache.find(faceNames[i]);
		if(iconsCache.end() != cachedImage)
		{
			std::map<std::string, QSize>::const_iterator cachedImageSize = iconSizeCache.find(faceNames[i]);
			if(cachedImageSize != iconSizeCache.end())
			{
				imageWidth = cachedImageSize->second.width();
				imageHeight = cachedImageSize->second.height();
			}
			
			painter->drawImage(QPoint(faceStartX + i * (FACE_IMAGE_SIZE + FACE_IMAGE_BORDER), y + FACE_IMAGE_BORDER), *cachedImage->second);
		}
	}
	
	//FACE SIZE
	QString sizeInfo = QString("%1x%2").arg(QString().setNum(imageWidth), QString().setNum(imageHeight));
	painter->setFont(QFont(FONT_NAMES[FACE_SIZE_ITEM_DATA], FONT_SIZES[FACE_SIZE_ITEM_DATA], FONT_STYLES[FACE_SIZE_ITEM_DATA]));
	fontMetrics = painter->fontMetrics();
	textRect = fontMetrics.boundingRect(description);
	painter->drawText(FACE_IMAGE_BORDER, textStartY + GetAdjustedTextHeight(textRect.height()), sizeInfo);
	
	// draw selected item
	if(option.state & QStyle::State_Selected)
	{
		QBrush br = option.palette.highlight();
		QColor cl = br.color();
		cl.setAlpha(SELECTION_COLOR_ALPHA);
		br.setColor(cl);
		painter->setBrush(br);
		painter->setPen(SELECTION_BORDER_COLOR);
		painter->drawRect(r);
	}
}

QSize CubeListItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	return QSize(option.rect.width(), itemHeight);
}