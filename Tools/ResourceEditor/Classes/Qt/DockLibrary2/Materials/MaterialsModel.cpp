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
#include <QMimeData>

const char * MaterialsModel::mimeFormatMaterial = "application/dava.nmaterial";

MaterialsModel::MaterialsModel(QObject * parent)
    : QStandardItemModel(parent)
    , rootMaterial(NULL)
{
}

MaterialsModel::~MaterialsModel()
{
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
            lodMaterials.push_back(materials[i]);
        }
    }
}

void MaterialsModel::RebuildModelFromMaterial()
{
    if(!rootMaterial) return;

    QStandardItem * rootItem = invisibleRootItem();
    AddMaterialToItem(rootMaterial, rootItem);
}

void MaterialsModel::RebuildModelFromAllMaterials()
{
    QStandardItem * rootItem = invisibleRootItem();
    for(int i = 0; i < (int)materials.size(); )
    {
        QStandardItem *item = new QStandardItem();
        
        DAVA::NMaterial *mat = materials[i];
        i += AddMaterialToItem(mat, item);
        
        rootItem->appendRow(item);
    }
    
    for(int i = 0; i < (int)lodMaterials.size(); ++i)
    {
        QStandardItem *item = new QStandardItem();
        
        DAVA::NMaterial *mat = materials[i];
        SetMaterialToItem(mat, item);
        
        rootItem->appendRow(item);
    }
}

int MaterialsModel::AddMaterialToItem(DAVA::NMaterial * material, QStandardItem * rootItem)
{
    SetMaterialToItem(material, rootItem);
    
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

void MaterialsModel::SetMaterialToItem(DAVA::NMaterial * material, QStandardItem * item)
{
    item->setText(GetName(material));
    item->setData(QVariant::fromValue<DAVA::NMaterial *>(material));
    item->setIcon(QIcon(QString::fromUtf8(":/QtIcons/materialeditor.png")));
    item->setEditable(false);
}


QString MaterialsModel::GetName(DAVA::NMaterial * material)
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

QMimeData * MaterialsModel::mimeData(const QModelIndexList & indexes) const
{
	QMimeData* ret = NULL;
    
	if(indexes.size() > 0)
	{
        QVector<void*> data;
        foreach(QModelIndex index, indexes)
        {
            data.push_back(GetMaterial(index));
        }
        
        ret = EncodeMimeData(data, mimeFormatMaterial);
    }
    
	return ret;
}

QStringList MaterialsModel::mimeTypes() const
{
	QStringList types;
    
	types << mimeFormatMaterial;
    
	return types;
}


QMimeData * MaterialsModel::EncodeMimeData(const QVector<void *> & data, const QString & format) const
{
	QMimeData *mimeData = NULL;
	
	if(data.size() > 0)
	{
		mimeData = new QMimeData();
		QByteArray encodedData;
        
		QDataStream stream(&encodedData, QIODevice::WriteOnly);
		for (int i = 0; i < data.size(); ++i)
		{
			stream.writeRawData((char *) &data[i], sizeof(void *));
		}
        
		mimeData->setData(format, encodedData);
	}
    
	return mimeData;
}

QVector<void *> * MaterialsModel::DecodeMimeData(const QMimeData * data, const QString & format) const
{
	QVector<void*> * ret = NULL;
    
	if(data->hasFormat(format))
	{
		void* object = NULL;
		QByteArray encodedData = data->data(format);
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
        
		ret = new QVector<void *>();
		while(!stream.atEnd())
		{
			stream.readRawData((char *) &object, sizeof(void *));
			ret->push_back(object);
		}
	}
    
	return ret;
}


