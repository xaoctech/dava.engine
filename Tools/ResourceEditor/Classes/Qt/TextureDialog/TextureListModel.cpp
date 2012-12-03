#include "TextureListModel.h"
#include <QPainter>
#include <QFileInfo>

TextureListModel::TextureListModel(QObject *parent /* = 0 */) 
	: QAbstractListModel(parent)
	, curSortMode(TextureListModel::SortByName)
{}

TextureListModel::~TextureListModel()
{
	clear();
}

int TextureListModel::rowCount(const QModelIndex & /* parent */) const
{
	return textureDescriptorsFiltredSorted.size();
}

QVariant TextureListModel::data(const QModelIndex &index, int role) const
{
	if(index.isValid())
	{
		const DAVA::TextureDescriptor *curTextureDescriptor = textureDescriptorsFiltredSorted[index.row()];

		switch(role)
		{
		case Qt::DisplayRole:
			return QVariant(QFileInfo(curTextureDescriptor->GetSourceTexturePathname().c_str()).fileName());
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
	DAVA::TextureDescriptor *desc = getDescriptor(index);

	if(index.isValid() && texturesAll.contains(desc))
	{
		ret = texturesAll[desc];
	}

	return ret;
}

DAVA::Texture* TextureListModel::getTexture(const DAVA::TextureDescriptor* descriptor) const
{
	DAVA::Texture *ret = NULL;

	if(texturesAll.contains(descriptor))
	{
		ret = texturesAll[descriptor];
	}

	return ret;
}

DAVA::TextureDescriptor* TextureListModel::getDescriptor(const QModelIndex &index) const
{
	DAVA::TextureDescriptor *ret = NULL;

	if(index.isValid() && textureDescriptorsFiltredSorted.size() > index.row())
	{
		ret = textureDescriptorsFiltredSorted[index.row()];
	}

	return ret;
}

void TextureListModel::setTexture(const DAVA::TextureDescriptor* descriptor, DAVA::Texture *texture)
{
	if(texturesAll.contains(descriptor))
	{
		texturesAll[descriptor] = texture;
	}
}

bool TextureListModel::isHighlited(const QModelIndex &index) const
{
	bool ret = false;
	DAVA::TextureDescriptor *descriptor = getDescriptor(index);

	if(NULL != descriptor)
	{
		ret = textureDescriptorsHighlight.contains(descriptor);
	}

	return ret;
}

void TextureListModel::dataReady(const DAVA::TextureDescriptor *desc)
{
	int i = textureDescriptorsFiltredSorted.indexOf((DAVA::TextureDescriptor * const) desc);
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

	clear();

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

void TextureListModel::setHighlight(DAVA::SceneNode *node)
{
	textureDescriptorsHighlight.clear();
	if(textureDescriptorsFiltredSorted.size() > 0)
	{
		textureDescriptorsHighlight.push_back(textureDescriptorsFiltredSorted[0]);
		emit dataChanged(createIndex(0, 0), createIndex(textureDescriptorsFiltredSorted.size() - 1, 1));
	}
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

					addTexture(material->GetTextureName((DAVA::Material::eTextureLevel) t), material->GetTexture((DAVA::Material::eTextureLevel) t));
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
					addTexture(landscape->GetTextureName((DAVA::LandscapeNode::eTextureLevel) t), landscape->GetTexture((DAVA::LandscapeNode::eTextureLevel) t));
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
						addTexture(ldata->lightmapName, ldata->lightmap);
					}
				}
			}
		}
	}
}

void TextureListModel::addTexture(const DAVA::String &descPath, DAVA::Texture *texture)
{
	if(DAVA::FileSystem::GetExtension(descPath) != DAVA::TextureDescriptor::GetDescriptorExtension())
	{
		return;
	}

	if(!descPath.empty())
	{
		bool alreadyInVector = false;

		// search if there is no the same descriptorPath
		for(int i = 0; i < textureDescriptorsAll.size(); ++i)
		{
			if(descPath == textureDescriptorsAll[i]->pathname)
			{
				alreadyInVector = true;
				break;
			}
		}

		// if there is no the same descriprot and this file exists
		if(!alreadyInVector && DAVA::FileSystem::Instance()->IsFile(descPath))
		{
			DAVA::TextureDescriptor * descriptor = DAVA::TextureDescriptor::CreateFromFile(descPath);

			if(NULL != descriptor)
			{
				// if there is no such texture in vector - add it
				if(-1 == textureDescriptorsAll.indexOf(descriptor))
				{
					textureDescriptorsAll.push_back(descriptor);

					if(NULL != descriptor)
					{
						texturesAll[descriptor] = texture;
					}
				}
			}
		}
	}
}

void TextureListModel::clear()
{
	texturesAll.clear();
	textureDescriptorsHighlight.clear();
	textureDescriptorsFiltredSorted.clear();

	for(int i = 0; i < textureDescriptorsAll.size(); ++i)
	{
		DAVA::SafeRelease(textureDescriptorsAll[i]);
	}

	textureDescriptorsAll.clear();
}

void TextureListModel::applyFilterAndSort()
{
	textureDescriptorsFiltredSorted.clear();

	for(int i = 0; i < (int) textureDescriptorsAll.size(); ++i)
	{
		if(curFilter.isEmpty() || DAVA::String::npos != textureDescriptorsAll[i]->pathname.find(curFilter.toStdString()))
		{
			textureDescriptorsFiltredSorted.push_back(textureDescriptorsAll[i]);
		}
	}

	switch(curSortMode)
	{
	case SortByName:
		std::sort(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), TextureListModel::sortFnByName);
		break;
	case SortBySize:
		std::sort(textureDescriptorsFiltredSorted.begin(), textureDescriptorsFiltredSorted.end(), TextureListModel::sortFnBySize);
		break;
	default:
		break;
	}
}

bool TextureListModel::sortFnByName(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2)
{
	return QFileInfo(t1->pathname.c_str()).completeBaseName() < QFileInfo(t2->pathname.c_str()).completeBaseName();
}

bool TextureListModel::sortFnBySize(const DAVA::TextureDescriptor* t1, const DAVA::TextureDescriptor* t2)
{
	return QFileInfo(t1->GetSourceTexturePathname().c_str()).size() < QFileInfo(t2->GetSourceTexturePathname().c_str()).size();
}
