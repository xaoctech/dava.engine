#include <QtGui>
#include "MaterialBrowser/MaterialTreeModel.h"

MaterialTreeModel::MaterialTreeModel(QObject *parent)
	: QAbstractItemModel(parent)
	, scene(NULL)
	, rootFiltredMaterial(NULL)
{ }

MaterialTreeModel::~MaterialTreeModel()
{
}

void MaterialTreeModel::SetScene(DAVA::Scene *scene)
{
	this->scene = scene;
	SearchMaterialsInScene();
}

inline const MaterialTreeItem* MaterialTreeModel::Item(const QModelIndex &index) const
{
	return (MaterialTreeItem*) index.internalPointer();
}

int MaterialTreeModel::columnCount(const QModelIndex &parent) const
{
	int count = 0;

	if(parent.isValid())
	{
		const MaterialTreeItem* item = Item(parent);
		count = item->ColumnCount();
	}
	else if(NULL != rootFiltredMaterial)
	{
		count = rootFiltredMaterial->ColumnCount();
	}

	return count;
}

int MaterialTreeModel::rowCount(const QModelIndex &parent) const
{
	int count = 0;

	if(parent.isValid())
	{
		const MaterialTreeItem* item = Item(parent);
		count = item->ChildCount();
	}
	else if(NULL != rootFiltredMaterial)
	{
		count = rootFiltredMaterial->ChildCount();
	}

	return count;
}


QVariant MaterialTreeModel::data(const QModelIndex &index, int role) const
{
	QVariant v;

	if (index.isValid())
	{
		const MaterialTreeItem* item = Item(index);

		switch(role)
		{
		case Qt::DisplayRole:
			v = item->Data(index.column());
			break;

		default:
			break;
		}
	}

	return v;
}

QModelIndex MaterialTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	QModelIndex index;

	if (hasIndex(row, column, parent))
	{
		const MaterialTreeItem* item;

		if (parent.isValid())
		{
			item = Item(index);
		}
		else
		{
			item = rootFiltredMaterial;
		}

		if(NULL != item)
		{
			MaterialTreeItem *childItem = item->Child(row);
			if(NULL != childItem)
			{
				index = createIndex(row, column, childItem);
			}
		}
	}

	return index;
}

QModelIndex MaterialTreeModel::parent(const QModelIndex &index) const
{
	QModelIndex parentIndex;

	if (index.isValid())
	{
		const MaterialTreeItem* item = Item(index);
		if(NULL != item && item != rootFiltredMaterial)
		{
			MaterialTreeItem *parent = item->Parent();

			if(NULL != parent)
			{
				parentIndex = createIndex(parent->Row(), 0, parent);
			}
		}
	}

	return parentIndex;
}

void MaterialTreeModel::SearchMaterialsInScene()
{
	if(NULL != scene)
	{
		allMaterials.clear();
		scene->GetDataNodes(allMaterials);

		ApplyFilterAndSort();
	}
}

void MaterialTreeModel::SearchMaterialsInFolder()
{

}

void MaterialTreeModel::ApplyFilterAndSort()
{
	DAVA::SafeDelete(rootFiltredMaterial);

	rootFiltredMaterial = new MaterialTreeItem("Root");
	MaterialTreeItem *rootFolder = new MaterialTreeItem("Materials", rootFiltredMaterial);

	// No actual filtering. Plain tree
	for(int i = 0; i < (int) allMaterials.size(); ++i)
	{
		DAVA::Material *material = allMaterials[i];

		if(NULL != material)
		{
			rootFolder->AddChild(new MaterialTreeItem(material));
		}
	}

	emit dataChanged(index(0, 0), index(rootFiltredMaterial->ChildCount(), rootFiltredMaterial->ColumnCount()));
}


