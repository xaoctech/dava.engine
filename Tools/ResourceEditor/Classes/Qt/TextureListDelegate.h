#ifndef __TEXTURE_LIST_DELEGATE_H__
#define __TEXTURE_LIST_DELEGATE_H__

#include <QAbstractItemDelegate>
#include <QFont>
#include <QFontMetrics>

class QPainter;

class TextureListDelegate : public QAbstractItemDelegate
{
	Q_OBJECT

public:
	TextureListDelegate(QObject *parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
	QFont nameFont;
	QFontMetrics nameFontMetrics;
	mutable QMap<QString, QImage> cachedImages;

	QImage getImage(const QString path) const;
};

#endif // __TEXTURE_LIST_DELEGATE_H__
