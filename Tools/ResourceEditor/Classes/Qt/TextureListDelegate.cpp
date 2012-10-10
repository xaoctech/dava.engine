#include "TextureListDelegate.h"
#include "TextureListModel.h"
#include <QPainter>

#define TEXTURE_PREVIEW_SIZE 100
#define BORDER_MARGIN 1
#define BORDER_COLOR QColor(0, 0, 0, 25)
#define SELECTION_BORDER_COLOR QColor(0, 0, 0, 50)
#define SELECTION_COLOR_ALPHA 100
#define INFO_TEXT_COLOR QColor(0, 0, 0, 100)

TextureListDelegate::TextureListDelegate(QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
	, nameFont("Arial", 10, QFont::Bold)
	, nameFontMetrics(nameFont)
{
};

void TextureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	const QAbstractItemModel *curModel = index.model();

	QVariant texturePath = curModel->data(index, TextureListModel::TexturePath);
	QVariant textureName = curModel->data(index, TextureListModel::TextureName);
	QVariant textureDimension = curModel->data(index, TextureListModel::TextureDimension);
	QVariant textureDataSize = curModel->data(index, TextureListModel::TextureDataSize);

	QImage img = getImage(texturePath.toString());

	painter->save();
	painter->setClipRect(option.rect);

	// draw border
	QRect borderRect = option.rect;
	borderRect.adjust(BORDER_MARGIN, BORDER_MARGIN, -BORDER_MARGIN, -BORDER_MARGIN);
	painter->setPen(BORDER_COLOR);
	painter->drawRect(borderRect);

	// draw image preview
	if(!img.isNull())
	{
		QSize imageSize = img.rect().size();
		imageSize.scale(QSize(TEXTURE_PREVIEW_SIZE - option.decorationSize.width(), TEXTURE_PREVIEW_SIZE - option.decorationSize.height()), Qt::KeepAspectRatio);
		int imageX =  option.rect.x() + (TEXTURE_PREVIEW_SIZE - imageSize.width())/2;
		int imageY =  option.rect.y() + (TEXTURE_PREVIEW_SIZE - imageSize.height())/2;
		painter->drawImage(QRect(QPoint(imageX, imageY), imageSize), img);
	}

	// draw text info
	{
		QRectF textRect = option.rect;
		textRect.adjust(TEXTURE_PREVIEW_SIZE, option.decorationSize.height() / 2, 0, 0);

		QFont origFont = painter->font();
		painter->setFont(nameFont);
		painter->drawText(textRect, textureName.toString());

		painter->setFont(origFont);
		painter->setPen(INFO_TEXT_COLOR);
		textRect.adjust(0, nameFontMetrics.height(), 0, 0);

		QString infoText;
		char dimen[32];

		sprintf(dimen, "%dx%d", textureDimension.toSize().width(), textureDimension.toSize().height());
		//infoText += "Dimension: ";
		infoText += dimen;
		infoText += "\nData size: ";
		infoText += textureDataSize.toString();

		painter->drawText(textRect, infoText);
	}

	// draw selected item
	if(option.state & QStyle::State_Selected)
	{
		QBrush br = option.palette.highlight();
		QColor cl = br.color();
		cl.setAlpha(SELECTION_COLOR_ALPHA);
		br.setColor(cl);
		painter->setBrush(br);
		painter->setPen(SELECTION_BORDER_COLOR);
		painter->drawRect(borderRect);
	}

	painter->restore();
}

QSize TextureListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(option.rect.x(), TEXTURE_PREVIEW_SIZE);
}

QImage TextureListDelegate::getImage(const QString path) const
{
	if(!cachedImages.contains(path))
	{
		cachedImages.insert(path, QImage(path));
	}

	return cachedImages.value(path);
}
