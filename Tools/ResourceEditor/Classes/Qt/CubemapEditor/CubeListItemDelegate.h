#ifndef __CUBE_LIST_ITEM_DELEGATE_H__
#define __CUBE_LIST_ITEM_DELEGATE_H__

#include <QPainter>
#include <QAbstractItemDelegate>

#include "Base/BaseTypes.h"

#define CUBELIST_DELEGATE_ITEMFULLPATH (Qt::UserRole)
#define CUBELIST_DELEGATE_ITEMFILENAME (Qt::UserRole + 1)

class CubeListItemDelegate : public QAbstractItemDelegate
{
	Q_OBJECT
	
protected:
	
	int itemHeight;
	std::map<std::string, QImage*> iconsCache;
	std::map<std::string, QSize> iconSizeCache;
	
private:
	
	int GetAdjustedTextHeight(int baseHeight) const;
	QRect GetCheckBoxRect(const QStyleOptionViewItem & option) const;
	
public:
	
	CubeListItemDelegate(QObject *parent = 0);
	virtual ~CubeListItemDelegate();
	
	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
		
	void ClearCache();
	void UpdateCache(QStringList& filesList);
	
signals:
	
	void OnEditCubemap(const QModelIndex &index);
	void OnItemCheckStateChanged(const QModelIndex &index);

};

#endif /* defined(__CUBE_LIST_ITEM_DELEGATE_H__) */
