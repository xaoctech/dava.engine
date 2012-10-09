#include "TextureListDelegate.h"
#include <QPainter>

#define TEXTURE_PREVIEW_SIZE 100

TextureListDelegate::TextureListDelegate(QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
{
};

void TextureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QVariant path = index.model()->data(index);
	QImage img = getImage(path.toString());

	painter->setClipRect(option.rect);
	//painter->save();

	if(!img.isNull())
	{
		QSize imageSize = img.rect().size();
		imageSize.scale(QSize(TEXTURE_PREVIEW_SIZE - option.decorationSize.width(), TEXTURE_PREVIEW_SIZE - option.decorationSize.height()), Qt::KeepAspectRatio);
		int imageX =  option.rect.x() + (TEXTURE_PREVIEW_SIZE - imageSize.width())/2;
		int imageY =  option.rect.y() + (TEXTURE_PREVIEW_SIZE - imageSize.height())/2;
		painter->drawImage(QRect(QPoint(imageX, imageY), imageSize), img);

		QRectF textRect = option.rect;
		textRect.adjust(TEXTURE_PREVIEW_SIZE, 0, 0, 0);
		painter->drawRect(textRect);
		painter->drawText(textRect, "test text\nline 2\nline 3");
	}

	if(option.state & QStyle::State_Selected)
	{
		QBrush br = option.palette.highlight();
		QColor cl = br.color();
		cl.setAlpha(150);
		br.setColor(cl);
		painter->setBrush(br);
		painter->drawRect(option.rect);
	}

	//painter->restore();
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
