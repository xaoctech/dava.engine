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
	DAVA::Vector<DAVA::Texture *> textures;

	void searchTexturesInMaterial(DAVA::Material *material);
	void searchTexturesInNodes(DAVA::SceneNode *parentNode);
	void addTexture(DAVA::Texture *texture);
};

#endif // __TEXTURE_LIST_MODEL_H__
