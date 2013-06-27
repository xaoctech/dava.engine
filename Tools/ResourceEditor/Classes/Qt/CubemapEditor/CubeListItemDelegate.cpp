#include "CubemapEditor/CubeListItemDelegate.h"
#include "CubemapEditor/CubemapUtils.h"
#include "Render/Texture.h"
#include "Render/TextureDescriptor.h"

#define FACE_IMAGE_SIZE 64
#define FACE_IMAGE_BORDER 5
#define LIST_ITEM_OFFSET 1
#define LIST_ITEMS_PER_PAGE 9

#define SELECTION_BORDER_COLOR QColor(0, 0, 0, 50)
#define SELECTION_COLOR_ALPHA 100

CubeListItemDelegate::CubeListItemDelegate(QObject *parent) : currentPage(0)
{
	
}

CubeListItemDelegate::~CubeListItemDelegate()
{
	
}

void CubeListItemDelegate::SetNeedsRepaint()
{
	
}

void CubeListItemDelegate::SetCurrentPage(int newPage)
{
	if(currentPage != newPage)
	{
		SetNeedsRepaint();
	}
	
	currentPage = newPage;
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
	
	//TITLE
	painter->setFont(QFont("Arial", 16, QFont::Bold));
	QFontMetrics fontMetrics = painter->fontMetrics();
	QRect textRect = fontMetrics.boundingRect(title);
	painter->drawText(FACE_IMAGE_BORDER, y + textRect.height() + FACE_IMAGE_BORDER, title);
	
	//DESCRIPTION
	painter->setFont(QFont("Arial", 11, QFont::Normal));
	painter->drawText(FACE_IMAGE_BORDER, y + 2 * (textRect.height() + FACE_IMAGE_BORDER), description);
	
	int imageWidth = 0;
	int imageHeight = 0;
	DAVA::Vector<DAVA::String> faceNames;
	CubemapUtils::GenerateFaceNames(description.toStdString(), faceNames);
	for(int i = 0; i < faceNames.size(); ++i)
	{
		//TODO: add icons caching
		QImage faceImage;
		faceImage.load(faceNames[i].c_str());
		
		//VI: all images should have the same size
		//VI: may be should add additional check here
		imageWidth = faceImage.width();
		imageHeight = faceImage.height();
		
		QImage scaledFace = faceImage.scaled(FACE_IMAGE_SIZE, FACE_IMAGE_SIZE);
		
		painter->drawImage(QPoint(faceStartX + i * (FACE_IMAGE_SIZE + FACE_IMAGE_BORDER), y + FACE_IMAGE_BORDER), scaledFace);
	}
	
	//FACE SIZE
	QString sizeInfo = QString("%1x%2").arg(QString().setNum(imageWidth), QString().setNum(imageHeight));
	int descriptionStartY = 2 * (textRect.height() + FACE_IMAGE_BORDER);
	painter->setFont(QFont("Arial", 11, QFont::Normal));
	fontMetrics = painter->fontMetrics();
	textRect = fontMetrics.boundingRect(description);
	painter->drawText(FACE_IMAGE_BORDER, y + descriptionStartY + textRect.height() + FACE_IMAGE_BORDER, sizeInfo);
	
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
	return QSize(option.rect.width(), (FACE_IMAGE_SIZE + 2 * FACE_IMAGE_BORDER + 2 * LIST_ITEM_OFFSET + 1));
}