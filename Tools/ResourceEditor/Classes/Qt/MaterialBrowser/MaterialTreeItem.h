#ifndef __MATERIAL_ITEM_H__
#define __MATERIAL_ITEM_H__

#include <QList>
#include <QVariant>

#include "DAVAEngine.h"

class MaterialTreeItem : public DAVA::BaseObject
{
public:
	typedef enum ItemType
	{
		TYPE_MATERIAL,
		TYPE_FOLDER
	} ItemType;

	MaterialTreeItem(DAVA::Material *material, MaterialTreeItem *parent = NULL);
	MaterialTreeItem(QString folderName, MaterialTreeItem *parent = NULL);
	~MaterialTreeItem();

	void AddChild(MaterialTreeItem *child);

	MaterialTreeItem *Child(int row) const;
	MaterialTreeItem *Parent() const;
	int Row() const;

	int ChildCount() const;
	int ColumnCount() const;

	QVariant Data(int column) const;
	ItemType Type() const;

	const DAVA::Material* Material() const;

private:
	MaterialTreeItem *parentItem;
	QList<MaterialTreeItem*> childItems;

	QString name;
	const DAVA::Material *material;
	ItemType type;
};

#endif // __MATERIAL_ITEM_H__
