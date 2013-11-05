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

#include <QStandardItem>

MaterialsModel::MaterialsModel(QObject *parent)
    : QStandardItemModel(parent)
    , rootMaterial(NULL)
{
}

MaterialsModel::~MaterialsModel()
{
    rootMaterial = NULL;
    materials.clear();
}

void MaterialsModel::SetScene(DAVA::Scene *scene)
{
    rootMaterial = NULL;
    materials.clear();
    
    if(scene)
    {
        const DAVA::RenderSystem * rSystem = scene->GetRenderSystem();
        const DAVA::MaterialSystem * matSystem = rSystem->GetMaterialSystem();
        
        matSystem->BuildMaterialList(NULL, materials);
    }
    
    RebuildModel();
}

void MaterialsModel::SetRootMaterial(DAVA::NMaterial *material)
{
    rootMaterial = material;
    RebuildModel();
}

DAVA::NMaterial * MaterialsModel::GetRootMaterial() const
{
    return rootMaterial;
}


void MaterialsModel::RebuildModel()
{
    //clear all data;
    QStandardItem *rootItem = invisibleRootItem();
    rootItem->removeRows(0, rowCount());
    
    if(rootMaterial)
    {
        AddMaterialToItem(rootMaterial, rootItem);
    }
    else
    {
        //build new model
        for(int i = 0; i < (int)materials.size(); )
        {
            QStandardItem *item = new QStandardItem();
            
            DAVA::NMaterial *mat = materials[i];
            i += AddMaterialToItem(mat, item);
            
            rootItem->appendRow(item);
        }
    }
}

int MaterialsModel::AddMaterialToItem(DAVA::NMaterial *material, QStandardItem *rootItem)
{
    rootItem->setText(GetName(material));
    rootItem->setData(QVariant::fromValue<DAVA::NMaterial *>(material));
    rootItem->setIcon(QIcon(QString::fromUtf8(":/QtIcons/materialeditor.png")));
    rootItem->setEditable(false);
    
    int counter = 1;
    for(int i = 0; i < (int)material->GetChildrenCount(); ++i)
    {
        QStandardItem *item = new QStandardItem();
        rootItem->appendRow(item);
        
        DAVA::NMaterial *mat = material->GetChild(i);
        counter += AddMaterialToItem(mat, item);
    }
    
    return counter;
}


QString MaterialsModel::GetName(DAVA::NMaterial *material)
{
    if(!material) return QString();
    
    QString name = material->GetMaterialName().c_str();
    if(!material->IsConfigMaterial())
    {
        name = QString(material->GetParentName().c_str()) + "." + name;
    }
    
    if(material->IsSwitchable())
    {
        name += "_[SW]";
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



