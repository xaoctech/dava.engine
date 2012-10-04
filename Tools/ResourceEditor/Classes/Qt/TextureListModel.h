#ifndef __TEXTURE_LIST_MODEL_H__
#define __TEXTURE_LIST_MODEL_H__

#include <QAbstractListModel>
#include "DAVAEngine.h"

class TextureListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	TextureListModel(QObject *parent = 0);

	void setScene(DAVA::Scene *scene);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	// QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
	DAVA::Scene *scene;
};


#include <QAbstractItemDelegate>

class QAbstractItemModel;
class QPainter;

class TextureListDelegate : public QAbstractItemDelegate
{
	Q_OBJECT

public:
	TextureListDelegate(QObject *parent = 0);

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const;

};

#endif // __TEXTURE_LIST_MODEL_H__
