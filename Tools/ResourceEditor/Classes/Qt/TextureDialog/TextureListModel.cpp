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
		case TextureName:
			return QVariant(QFileInfo(curTexture->GetPathname().c_str()).fileName());
			break;

		case TexturePath:
			return QVariant(curTexture->GetPathname().c_str());
			break;

		case TextureDimension:
			return QVariant(QSize(curTexture->GetWidth(), curTexture->GetHeight()));
			break;

		case TextureDataSize:
			return QVariant(curTexture->GetDataSize());
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

	if(index.isValid())
	{
		ret = texturesFiltredSorted[index.row()];
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

	if(NULL != scene)
	{
		// Parse scene and find it all Textures

		// Search textures in materials
		DAVA::Vector<DAVA::Material *> allMaterials;
		scene->GetDataNodes(allMaterials);
		for(int i = 0; i < (int) allMaterials.size(); ++i)
		{
			searchTexturesInMaterial(allMaterials[i]);
		}

		// Search textures in scene nodes
		searchTexturesInNodes(scene);
	}

	applyFilterAndSort();

	endResetModel();
}

void TextureListModel::searchTexturesInMaterial(DAVA::Material *material)
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

void TextureListModel::searchTexturesInNodes(DAVA::SceneNode *parentNode)
{
	int count = parentNode->GetChildrenCount();
	for(int n = 0; n < count; ++n)
	{
		DAVA::SceneNode *childNode = parentNode->GetChild(n);
		DAVA::LandscapeNode *landscape = dynamic_cast<DAVA::LandscapeNode*>(childNode);

		if (landscape) 
		{
			for(int t = 0; t < DAVA::LandscapeNode::TEXTURE_COUNT; ++t)
			{
				addTexture(landscape->GetTexture((DAVA::LandscapeNode::eTextureLevel) t));
			}
		}

		searchTexturesInNodes(childNode);
	}
}

void TextureListModel::addTexture(DAVA::Texture *texture)
{
	if(NULL != texture && !texture->isRenderTarget && !texture->GetPathname().empty())
	{
		texturesAll.push_back(texture);
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
