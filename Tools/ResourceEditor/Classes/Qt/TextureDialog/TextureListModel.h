#ifndef __TEXTURE_LIST_MODEL_H__
#define __TEXTURE_LIST_MODEL_H__

#include <QAbstractListModel>
#include <QVector>
#include "DAVAEngine.h"

class TextureListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum TextureListSortMode
	{
		SortByName,
		SortBySize
	};

	TextureListModel(QObject *parent = 0);

	void setScene(DAVA::Scene *scene);
	void setFilter(QString filter);
	void setSortMode(TextureListModel::TextureListSortMode sortMode);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	void dataReady(const DAVA::TextureDescriptor *descriptor);

	DAVA::Texture* getTexture(const QModelIndex &index) const;
	DAVA::TextureDescriptor* getDescriptor(const QModelIndex &index) const;

private:
	DAVA::Scene *scene;
	QVector<DAVA::TextureDescriptor *> textureDescriptorsAll;
	QVector<DAVA::TextureDescriptor *> textureDescriptorsFiltredSorted;
	QMap<DAVA::TextureDescriptor *, DAVA::Texture *> texturesAll;

	TextureListSortMode curSortMode;
	QString	curFilter;

	void applyFilterAndSort();

	void searchTexturesInMaterial(DAVA::SceneNode *parentNode);
	void searchTexturesInLandscapes(DAVA::SceneNode *parentNode);
	void searchTexturesInMesh(DAVA::SceneNode *parentNode);
	void addTexture(const DAVA::String &descPath, DAVA::Texture *texture);

	static bool sortFnByName(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2);
	static bool sortFnBySize(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2);
};

#endif // __TEXTURE_LIST_MODEL_H__
