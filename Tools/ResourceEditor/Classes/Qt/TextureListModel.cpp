#include "TextureListModel.h"
#include <QPainter>

TextureListModel::TextureListModel(QObject *parent /* = 0 */) 
	: QAbstractListModel(parent)
{

}

int TextureListModel::rowCount(const QModelIndex & /* parent */) const
{
	return texturesFiltred.size();
}

QVariant TextureListModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	return QVariant(texturesFiltred[index.row()]->GetPathname().c_str());
}

void TextureListModel::setFilter(QString filter)
{
	beginResetModel();

	texturesFiltred.clear();

	for(int i = 0; i < (int) texturesAll.size(); ++i)
	{
		if(filter.isEmpty() || DAVA::String::npos != texturesAll[i]->GetPathname().find(filter.toStdString()))
		{
			texturesFiltred.push_back(texturesAll[i]);
		}
		else
		{
			QString s(texturesAll[i]->GetPathname().c_str());
		}
	}

	endResetModel();
}

void TextureListModel::setScene(DAVA::Scene *scene)
{
	beginResetModel();

	texturesAll.clear();

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

	texturesFiltred = texturesAll;

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
	if(NULL != texture /*&& texture->isRenderTarget*/)
	{
		texturesAll.push_back(texture);
	}
}
