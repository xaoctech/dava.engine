/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "MaterialModel.h"
#include "MaterialItem.h"

#include "Scene/SceneEditor2.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Systems/MaterialSystem.h"

MaterialModel::MaterialModel(QObject * parent)
    : QStandardItemModel(parent)
{ 
	QStringList headerLabels;
	headerLabels.append("Materials hierarchy");
	setHorizontalHeaderLabels(headerLabels);
}

MaterialModel::~MaterialModel()
{ }

void MaterialModel::SetScene(SceneEditor2 *scene)
{
	removeRows(0, rowCount());

	if(NULL != scene)
	{
		QStandardItem *root = invisibleRootItem();
		DAVA::MaterialSystem *matSys = scene->GetMaterialSystem();

		DAVA::Vector<DAVA::NMaterial *> materials;
		matSys->BuildMaterialList(scene, DAVA::NMaterial::MATERIALTYPE_MATERIAL, materials);

		for(DAVA::uint32 i = 0; i < (DAVA::uint32)materials.size(); ++i)
		{
			MaterialItem *item = new MaterialItem(materials[i]);
			root->appendRow(item);
		}
	}
}

DAVA::NMaterial * MaterialModel::GetMaterial(const QModelIndex & index) const
{
    if(!index.isValid()) return NULL;
    
	MaterialItem *item = (MaterialItem *) itemFromIndex(index);
    return item->GetMaterial();
}

QModelIndex MaterialModel::GetIndex(DAVA::NMaterial *material, const QModelIndex &parent) const
{
	QModelIndex ret = QModelIndex();

	MaterialItem* item = (MaterialItem*)itemFromIndex(parent);
	if(NULL != item && item->GetMaterial() == material)
	{
		ret = parent;
	}
	else
	{
		QStandardItem *sItem = (NULL != item) ? item : invisibleRootItem();
		if(NULL != sItem)
		{
			for(int i = 0; i < sItem->rowCount(); ++i)
			{
				ret = GetIndex(material, index(i, 0, parent));
				if(ret.isValid())
				{
					break;
				}
			}
		}
	}

	return ret;
}

QMimeData * MaterialModel::mimeData(const QModelIndexList & indexes) const
{
	if(indexes.size() > 0)
	{
        QVector<DAVA::NMaterial*> data;
        foreach(QModelIndex index, indexes)
        {
            data.push_back(GetMaterial(index));
        }
        
        return MimeDataHelper2<DAVA::NMaterial>::EncodeMimeData(data);
    }
    
	return NULL;
}

QStringList MaterialModel::mimeTypes() const
{
	QStringList types;
    
	types << MimeDataHelper2<DAVA::NMaterial>::GetMimeType();
    
	return types;
}

bool MaterialModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	bool ret = false;
	QVector<DAVA::NMaterial *> materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);

	if(materials.size() > 0)
	{
		if(dropCanBeAccepted(data, action, row, column, parent))
		{
			DAVA::NMaterial *targetMaterial = GetMaterial(parent);
			MaterialItem *targetMaterialItem = (MaterialItem *) itemFromIndex(parent);

			for(int i = 0; i < materials.size(); ++i)
			{
				MaterialItem *sourceMaterialItem = (MaterialItem *) itemFromIndex(GetIndex(materials[i]));
				if(NULL != sourceMaterialItem)
				{
					// TODO:
					// this should be done with command
					// -->
					materials[i]->GetParent()->RemoveChild(materials[i]);
					targetMaterial->AddChild(materials[i]);
					// <--

					((MaterialItem *) sourceMaterialItem->parent())->Sync();
				}
			}

			targetMaterialItem->Sync();
		}
	}

	return ret;
}

bool MaterialModel::dropCanBeAccepted(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	bool ret = false;
	QVector<DAVA::NMaterial *> materials = MimeDataHelper2<DAVA::NMaterial>::DecodeMimeData(data);

	if(materials.size() > 0)
	{
		// allow only direct drop to parent
		if(row == -1 && column == -1)
		{
			bool foundInacceptable = false;
			DAVA::NMaterial *targetMaterial = GetMaterial(parent);

			// allow drop only into material, but not into instance
			if(targetMaterial->GetMaterialType() == DAVA::NMaterial::MATERIALTYPE_MATERIAL)
			{
				// only instance type should be in mime data
				for(int i = 0; i < materials.size(); ++i)
				{
					if(materials[i]->GetMaterialType() != DAVA::NMaterial::MATERIALTYPE_INSTANCE)
					{
						foundInacceptable = true;
						break;
					}
				}
			}

			ret = !foundInacceptable;
		}
	}

	return ret;
}
