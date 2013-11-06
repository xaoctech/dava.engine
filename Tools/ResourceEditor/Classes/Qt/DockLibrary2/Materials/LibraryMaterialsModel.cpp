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



#include "LibraryMaterialsModel.h"
#include "LibraryMaterialsFilteringModel.h"

#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

#include "MaterialsModel.h"

#include <QMenu>
#include <QAction>

LibraryMaterialsModel::LibraryMaterialsModel()
    : LibraryBaseModel("Materials")
{
    treeModel = new MaterialsModel(this);
    listModel = new MaterialsModel(this);
    filteringModel = new LibraryMaterialsFilteringModel(this);
	filteringModel->SetModel(listModel);

	CreateActions();
    
    QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
    QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));
}

void LibraryMaterialsModel::TreeItemSelected(const QItemSelection & selection)
{
    const QModelIndex index = selection.indexes().first();
    
    DAVA::NMaterial * material = ((MaterialsModel *)treeModel)->GetMaterial(index);
    
    ((MaterialsModel *)listModel)->SetRootMaterial(material);
	filteringModel->invalidate();
}

void LibraryMaterialsModel::ListItemSelected(const QItemSelection & selection)
{
	QItemSelection listSelection = filteringModel->mapSelectionToSource(selection);
}


void LibraryMaterialsModel::SetProjectPath(const QString & path)
{
}

const QModelIndex LibraryMaterialsModel::GetTreeRootIndex() const
{
    return ((MaterialsModel *)treeModel)->index(-1, -1);
}

const QModelIndex LibraryMaterialsModel::GetListRootIndex() const
{
    const QModelIndex index = ((MaterialsModel *)listModel)->index(-1, -1);
    return filteringModel->mapFromSource(index);
}

bool LibraryMaterialsModel::PrepareTreeContextMenu(QMenu &contextMenu, const QModelIndex &index) const
{
    DAVA::NMaterial * material = ((MaterialsModel *)treeModel)->GetMaterial(index);

    return PrepareContextMenu(contextMenu, material);
}

bool LibraryMaterialsModel::PrepareListContextMenu(QMenu &contextMenu, const QModelIndex &index) const
{
    const QModelIndex listIndex = filteringModel->mapToSource(index);
    
    DAVA::NMaterial * material = ((MaterialsModel *)listModel)->GetMaterial(listIndex);
    return PrepareContextMenu(contextMenu, NULL);
}


void LibraryMaterialsModel::CreateActions()
{
}

void LibraryMaterialsModel::SceneActivated(SceneEditor2 *scene)
{
    ((MaterialsModel *)treeModel)->SetScene(scene);
    ((MaterialsModel *)listModel)->SetScene(scene);
    filteringModel->invalidate();
}

void LibraryMaterialsModel::SceneDeactivated(SceneEditor2 *scene)
{
    ((MaterialsModel *)treeModel)->SetScene(NULL);
    ((MaterialsModel *)listModel)->SetScene(NULL);
	filteringModel->invalidate();
}

bool LibraryMaterialsModel::PrepareContextMenu(QMenu &contextMenu, DAVA::NMaterial *material) const
{
    QVariant materialAsVariant = QVariant::fromValue<DAVA::NMaterial *>(material);

    
    QAction * actionEdit = contextMenu.addAction("Edit Material", this, SLOT(OnEdit()));
    actionEdit->setData(materialAsVariant);
    
    return true;
}

void LibraryMaterialsModel::OnEdit()
{
    QVariant materialAsVariant = ((QAction *)sender())->data();
    DAVA::NMaterial * material = materialAsVariant.value<DAVA::NMaterial *>();
}


