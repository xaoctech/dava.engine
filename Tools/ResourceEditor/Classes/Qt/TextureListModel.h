#ifndef __TEXTURE_LIST_MODEL_H__
#define __TEXTURE_LIST_MODEL_H__

#include <QAbstractListModel>
#include "DAVAEngine.h"

class TextureListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum TextureListSortMode
	{
		SORT_BY_NAME,
		SORT_BY_SIZE
	};

	TextureListModel(QObject *parent = 0);

	void setScene(DAVA::Scene *scene);
	void setFilter(QString filter);
	void setSortMode(TextureListModel::TextureListSortMode sortMode);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	// QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
	DAVA::Scene *scene;
	DAVA::Vector<DAVA::Texture *> texturesAll;
	DAVA::Vector<DAVA::Texture *> texturesFiltred;

	void searchTexturesInMaterial(DAVA::Material *material);
	void searchTexturesInNodes(DAVA::SceneNode *parentNode);
	void addTexture(DAVA::Texture *texture);
};

#endif // __TEXTURE_LIST_MODEL_H__
