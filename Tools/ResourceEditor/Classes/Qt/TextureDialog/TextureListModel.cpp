#include "TextureListModel.h"
#include <QPainter>
#include <QFileInfo>

TextureListModel::TextureListModel(QObject *parent /* = 0 */) 
	: QAbstractListModel(parent)
	, curSortMode(TextureListModel::SortByName)
{}

int TextureListModel::rowCount(const QModelIndex & /* parent */) const
{
	return texturesFiltredSorted.size();
}

QVariant TextureListModel::data(const QModelIndex &index, int role) const
{
	if(index.isValid())
	{
		const DAVA::Texture *curTexture = texturesFiltredSorted[index.row()];

		switch(role)
		{
		case Qt::DisplayRole:
			return QVariant(QFileInfo(curTexture->GetPathname().c_str()).fileName());
			break;

		default:
			break;
		}
	}

	return QVariant();
}

DAVA::Texture* TextureListModel::getTexture(const QModelIndex &index) const
{
	DAVA::Texture *ret = NULL;

	if(index.isValid() && texturesFiltredSorted.size() > index.row())
	{
		ret = texturesFiltredSorted[index.row()];
	}

	return ret;
}

DAVA::TextureDescriptor* TextureListModel::getDescriptor(const QModelIndex &index) const
{
	DAVA::TextureDescriptor *ret = NULL;
	DAVA::Texture *tex = getTexture(index);

	if(index.isValid() && textureDescriptors.contains(tex))
	{
		ret = textureDescriptors[tex];
	}

	return ret;
}

void TextureListModel::dataReady(const DAVA::Texture *texture)
{
	int i = texturesFiltredSorted.indexOf((DAVA::Texture * const) texture);
	emit dataChanged(this->index(i), this->index(i));
}

void TextureListModel::setFilter(QString filter)
{
	beginResetModel();
	curFilter = filter;
	applyFilterAndSort();
	endResetModel();
}

void TextureListModel::setSortMode(TextureListModel::TextureListSortMode sortMode)
{
	beginResetModel();
	curSortMode = sortMode;
	applyFilterAndSort();
	endResetModel();
}

void TextureListModel::setScene(DAVA::Scene *scene)
{
	beginResetModel();

	texturesAll.clear();
	texturesFiltredSorted.clear();

	// Parse scene and find it all Textures
	{
		// Search textures in materials
		searchTexturesInMaterial(scene);

		// Search textures in landscapes
		searchTexturesInLandscapes(scene);

		// Search textures in mesh
		searchTexturesInMesh(scene);
	}

	applyFilterAndSort();

	endResetModel();
}

void TextureListModel::searchTexturesInMaterial(DAVA::SceneNode *parentNode)
{
	if(NULL != parentNode)
	{
		DAVA::Vector<DAVA::Material *> allMaterials;

		parentNode->GetDataNodes(allMaterials);
		for(int i = 0; i < (int) allMaterials.size(); ++i)
		{
			DAVA::Material *material = allMaterials[i];

			if(NULL != material)
			{
				for(int t = 0; t < DAVA::Material::TEXTURE_COUNT; ++t)
				{
					if(material->type == DAVA::Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP &&
						t > DAVA::Material::TEXTURE_DIFFUSE)
					{
						continue;
					}

					addTexture(material->GetTexture((DAVA::Material::eTextureLevel) t));
				}
			}
		}
	}
}

void TextureListModel::searchTexturesInLandscapes(DAVA::SceneNode *parentNode)
{
	if(NULL != parentNode)
	{
		DAVA::Vector<DAVA::LandscapeNode *> allLandscapes;

		parentNode->GetChildNodes(allLandscapes);
		for(int i = 0; i < (int) allLandscapes.size(); ++i)
		{
			DAVA::LandscapeNode *landscape = allLandscapes[i];

			if(NULL != landscape)
			{
				for(int t = 0; t < DAVA::LandscapeNode::TEXTURE_COUNT; ++t)
				{
					addTexture(landscape->GetTexture((DAVA::LandscapeNode::eTextureLevel) t));
				}
			}
		}
	}
}

void TextureListModel::searchTexturesInMesh(DAVA::SceneNode *parentNode)
{
	if(NULL != parentNode)
	{
		DAVA::Vector<DAVA::MeshInstanceNode *> allMeshes;

		parentNode->GetChildNodes(allMeshes);
		for(int i = 0; i < (int) allMeshes.size(); ++i)
		{
			DAVA::MeshInstanceNode *mesh = allMeshes[i];

			if(NULL != mesh)
			{
				for(int t = 0; t < mesh->GetLightmapCount(); ++t)
				{
					DAVA::MeshInstanceNode::LightmapData *ldata = mesh->GetLightmapDataForIndex(t);
					if(NULL != ldata)
					{
						addTexture(ldata->lightmap);
					}
				}
			}
		}
	}
}

void TextureListModel::addTexture(DAVA::Texture *texture)
{
	if(NULL != texture && !texture->isRenderTarget && !texture->GetPathname().empty())
	{
		// if there is no such texture in vector - add it
		if(-1 == texturesAll.indexOf(texture))
		{
			texturesAll.push_back(texture);

			DAVA::TextureDescriptor * descriptor = texture->CreateDescriptor();
			if(NULL != descriptor)
			{
				textureDescriptors[texture] = descriptor;
			}
		}
	}
}

void TextureListModel::applyFilterAndSort()
{
	texturesFiltredSorted.clear();

	for(int i = 0; i < (int) texturesAll.size(); ++i)
	{
		if(curFilter.isEmpty() || DAVA::String::npos != texturesAll[i]->GetPathname().find(curFilter.toStdString()))
		{
			texturesFiltredSorted.push_back(texturesAll[i]);
		}
		else
		{
			QString s(texturesAll[i]->GetPathname().c_str());
		}
	}

	switch(curSortMode)
	{
	case SortByName:
		std::sort(texturesFiltredSorted.begin(), texturesFiltredSorted.end(), TextureListModel::sortFnByName);
		break;
	case SortBySize:
		std::sort(texturesFiltredSorted.begin(), texturesFiltredSorted.end(), TextureListModel::sortFnBySize);
		break;
	default:
		break;
	}
}

bool TextureListModel::sortFnByName(const DAVA::Texture* t1, const DAVA::Texture* t2)
{
	return QFileInfo(t1->GetPathname().c_str()).completeBaseName() < QFileInfo(t2->GetPathname().c_str()).completeBaseName();
}

bool TextureListModel::sortFnBySize(const DAVA::Texture* t1, const DAVA::Texture* t2)
{
	return (t1->GetDataSize() < t2->GetDataSize());
}
