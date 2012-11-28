#ifndef __TEXTURE_LIST_DELEGATE_H__
#define __TEXTURE_LIST_DELEGATE_H__

#include <QAbstractItemDelegate>
#include <QMap>
#include <QFont>
#include <QFontMetrics>

#include "DAVAEngine.h"

class QPainter;

class TextureListDelegate : public QAbstractItemDelegate
{
	Q_OBJECT

public:
	enum DrawRure
	{
		DRAW_PREVIEW_BIG,
		DRAW_PREVIEW_SMALL
	};

	TextureListDelegate(QObject *parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void setDrawRule(DrawRure rule);

private slots:
	void textureReadyOriginal(const DAVA::TextureDescriptor *descriptor, const QImage &image);

private:
	QFont nameFont;
	QFontMetrics nameFontMetrics;
	mutable QMap<const DAVA::TextureDescriptor *, QModelIndex> descriptorIndexes;

	DrawRure drawRule;

	void drawPreviewBig(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void drawPreviewSmall(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void drawFormatInfo(QPainter *painter, QRect rect, const DAVA::TextureDescriptor *descriptor) const;
};

#endif // __TEXTURE_LIST_DELEGATE_H__
