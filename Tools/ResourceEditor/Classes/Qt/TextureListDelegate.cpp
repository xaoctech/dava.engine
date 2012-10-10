#include "TextureListDelegate.h"
#include <QPainter>

#define TEXTURE_PREVIEW_SIZE 100
#define BORDER_MARGIN 1
#define BORDER_COLOR QColor(0, 0, 0, 25)
#define SELECTION_BORDER_COLOR QColor(0, 0, 0, 50)
#define SELECTION_COLOR_ALPHA 100

TextureListDelegate::TextureListDelegate(QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
{
};

void TextureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QVariant path = index.model()->data(index);
	QImage img = getImage(path.toString());

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
	QRectF textRect = option.rect;
	textRect.adjust(TEXTURE_PREVIEW_SIZE, 0, option.decorationSize.height() / 2, 0);
	painter->drawText(textRect, "test text\nline 2\nline 3");

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
