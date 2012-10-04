#include "TextureListModel.h"
#include <QPainter>

TextureListModel::TextureListModel(QObject *parent /* = 0 */) 
	: QAbstractListModel(parent)
{

}

int TextureListModel::rowCount(const QModelIndex & /* parent */) const
{
	return 10;
}

QVariant TextureListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	return QVariant(index.row());
}


TextureListDelegate::TextureListDelegate(QObject *parent /* = 0 */)
	: QAbstractItemDelegate(parent)
{
	
};

void TextureListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (option.state & QStyle::State_Selected)
		painter->fillRect(option.rect, option.palette.highlight());

	painter->setPen(Qt::blue);
	painter->drawRoundedRect(option.rect, 5, 5);
}

QSize TextureListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(option.rect.x(), 50);
}
