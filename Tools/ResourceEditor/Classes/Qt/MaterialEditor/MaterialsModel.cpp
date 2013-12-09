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



#include "MaterialsModel.h"
#include "MaterialsItem.h"

#include "Scene/SceneEditor2.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include "Scene3D/Scene.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/MaterialSystem.h"

MaterialsModel::MaterialsModel(QObject * parent)
    : QStandardItemModel(parent)
    , curScene(NULL)
{
}

MaterialsModel::~MaterialsModel()
{
    switchableMaterials.clear();
    curScene = NULL;
}

void MaterialsModel::RemoveAllItems()
{
    QStandardItem * rootItem = invisibleRootItem();
    rootItem->removeRows(0, rowCount());
}

void MaterialsModel::SetScene(SceneEditor2 *scene)
{
    switchableMaterials.clear();

    curScene = scene;

    RetriveMaterials();
    
    RebuildModel();
}

void MaterialsModel::RetriveMaterials()
{
    if(!curScene) return;
    
    DAVA::MaterialSystem *system = curScene->renderSystem->GetMaterialSystem();
    DAVA::Vector<DAVA::NMaterial *> materials;
    system->BuildMaterialList(NULL, materials);
    for(DAVA::uint32 i = 0; i < (DAVA::uint32)materials.size(); ++i)
    {
        if(materials[i]->IsSwitchable() && materials[i]->IsConfigMaterial())
        {
            switchableMaterials.push_back(materials[i]);
        }
    }
}

void MaterialsModel::RebuildModel()
{
    RemoveAllItems();

    QStandardItem * rootItem = invisibleRootItem();
    for(DAVA::uint32 i = 0; i < (DAVA::uint32)switchableMaterials.size(); ++i)
    {
        MaterialsItem *item = new MaterialsItem(switchableMaterials[i], this);
        AddMaterialToItem(switchableMaterials[i], item);

        rootItem->appendRow(item);
    }
}

void MaterialsModel::AddMaterialToItem(DAVA::NMaterial * material, MaterialsItem * rootItem)
{
    for(DAVA::int32 i = 0; i < material->GetChildrenCount(); ++i)
    {
        DAVA::NMaterial *mat = material->GetChild(i);

        MaterialsItem *item = new MaterialsItem(mat, this);
        AddMaterialToItem(mat, item);

        rootItem->appendRow(item);
    }
}


DAVA::NMaterial * MaterialsModel::GetMaterial(const QModelIndex & index) const
{
    if(!index.isValid()) return NULL;
    
    QStandardItem *item = itemFromIndex(index);
    
    DAVA::NMaterial *material = item->data().value<DAVA::NMaterial *>();
    return material;
}

QMimeData * MaterialsModel::mimeData(const QModelIndexList & indexes) const
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

QStringList MaterialsModel::mimeTypes() const
{
	QStringList types;
    
	types << MimeDataHelper2<DAVA::NMaterial>::GetSupportedTypeName();
    
	return types;
}
