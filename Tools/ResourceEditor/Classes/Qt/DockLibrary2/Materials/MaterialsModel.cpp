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

#include "Scene/EntityGroup.h"
#include "Tools/MimeData/MimeDataHelper2.h"

#include <QMimeData>
#include <QStandardItem>

MaterialsModel::MaterialsModel(QObject * parent)
    : QStandardItemModel(parent)
    , rootMaterial(NULL)
{
}

MaterialsModel::~MaterialsModel()
{
    selectedMaterials.clear();
    Clear();
}

void MaterialsModel::Clear()
{
    rootMaterial = NULL;

    materials.clear();
    lodMaterials.clear();
    
    QStandardItem * rootItem = invisibleRootItem();
    rootItem->removeRows(0, rowCount());
}

void MaterialsModel::SetScene(DAVA::Scene * scene)
{
    selectedMaterials.clear();
    Clear();
    
    if(scene)
    {
        const DAVA::RenderSystem * rSystem = scene->GetRenderSystem();
        const DAVA::MaterialSystem * matSystem = rSystem->GetMaterialSystem();
        
        matSystem->BuildMaterialList(NULL, materials);
    }
    
    PrepareLodMaterials();
    RebuildModelFromAllMaterials();
}

void MaterialsModel::SetRootMaterial(DAVA::NMaterial * material)
{
    Clear();
    
    rootMaterial = material;
    BuildMaterialsFromRootRecursive(rootMaterial);
    
    RebuildModelFromMaterial();
}

DAVA::NMaterial * MaterialsModel::GetRootMaterial() const
{
    return rootMaterial;
}

void MaterialsModel::PrepareLodMaterials()
{
    if(rootMaterial != NULL) return;
    
    for(int i = 0; i < (int)materials.size(); ++i)
    {
        if(materials[i]->IsSwitchable() && NULL != materials[i]->GetParent())
        {
            lodMaterials[materials[i]->GetMaterialName().c_str()] = materials[i];
        }
    }
}

void MaterialsModel::RebuildModelFromMaterial()
{
    if(!rootMaterial) return;

    AddMaterialToItem(rootMaterial, (MaterialsItem *)invisibleRootItem());
}

void MaterialsModel::RebuildModelFromAllMaterials()
{
    QStandardItem * rootItem = invisibleRootItem();
    for(int i = 0; i < (int)materials.size(); )
    {
        MaterialsItem *item = new MaterialsItem(materials[i], this);
        i += AddMaterialToItem(materials[i], item);
        
        rootItem->appendRow(item);
    }
    
    
    auto endIt = lodMaterials.end();
    for(auto it = lodMaterials.begin(); it != endIt; ++it)
    {
        MaterialsItem *item = new MaterialsItem(it->second, this);
        rootItem->appendRow(item);
    }
}

void MaterialsModel::SceneStructureChanged(DAVA::Scene * scene)
{
    DAVA::NMaterial *material = rootMaterial;

    selectedMaterials.clear();
    Clear();
    
    if(scene)
    {
        const DAVA::RenderSystem * rSystem = scene->GetRenderSystem();
        const DAVA::MaterialSystem * matSystem = rSystem->GetMaterialSystem();
        
        matSystem->BuildMaterialList(NULL, materials);
    }
    
    if(material)
    {
        for(int i = 0; i < (int)materials.size(); ++i)
        {
            if(material == materials[i])
            {
                SetRootMaterial(material);
                return;
            }
        }
        
    }
    
    PrepareLodMaterials();
    RebuildModelFromAllMaterials();
}

int MaterialsModel::AddMaterialToItem(DAVA::NMaterial * material, MaterialsItem * rootItem)
{
    int counter = 1;
    for(int i = 0; i < (int)material->GetChildrenCount(); ++i)
    {
        DAVA::NMaterial *mat = material->GetChild(i);

        MaterialsItem *item = new MaterialsItem(mat, this);
        counter += AddMaterialToItem(mat, item);

        rootItem->appendRow(item);
    }
    
    return counter;
}

QString MaterialsModel::GetName(DAVA::NMaterial * material)
{
    if(!material) return QString();
    
    QString name = material->GetMaterialName().c_str();
    if(!material->IsConfigMaterial())
    {
        name = QString(material->GetParentName().c_str()) + "." + name;
    }
    
    return name;
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

void MaterialsModel::SetSelection(const EntityGroup & selected)
{
    selectedMaterials.clear();
    
    for(size_t i = 0; i < selected.Size(); ++i)
    {
        RetrieveMaterialRecursive(selected.GetEntity(i));
    }
}

void MaterialsModel::RetrieveMaterialRecursive(DAVA::Entity *entity)
{
    for(DAVA::int32 i = 0; i < entity->GetChildrenCount(); ++i)
    {
        RetrieveMaterialRecursive(entity->GetChild(i));
    }
    
    
    DAVA::RenderObject *ro = DAVA::GetRenderObject(entity);
    if(!ro) return;
    
    DAVA::uint32 count = ro->GetRenderBatchCount();
    for(DAVA::uint32 b = 0; b < count; ++b)
    {
        DAVA::RenderBatch *renderBatch = ro->GetRenderBatch(b);
        
        DAVA::NMaterial *material = renderBatch->GetMaterial();
        
        while (material)
        {
            selectedMaterials.insert(material);
            material = material->GetParent();
        }
    }
}

void MaterialsModel::BuildMaterialsFromRootRecursive(DAVA::NMaterial *root)
{
    materials.push_back(root);
    
    for(DAVA::int32 i = 0; i < root->GetChildrenCount(); ++i)
    {
        BuildMaterialsFromRootRecursive(root->GetChild(i));
    }
}



bool MaterialsModel::IsMaterialSelected(DAVA::NMaterial * material) const
{
    return (selectedMaterials.find(material) != selectedMaterials.end());
}
