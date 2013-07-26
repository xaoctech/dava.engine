/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "DockSceneTree/SceneTree.h"
#include "Main/mainwindow.h"
#include "StringConstants.h"
#include <QBoxLayout>
#include <QDropEvent>
#include <QMenu>
#include <QFileDialog>

#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/SceneValidator.h"
#include "Main/QTUtils.h"

// framework
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

// commands
#include "Commands2/ParticleEditorCommands.h"

SceneTree::SceneTree(QWidget *parent /*= 0*/)
	: QTreeView(parent)
	, skipTreeSelectionProcessing(false)
{

	treeModel = new SceneTreeModel();
	filteringProxyModel = new SceneTreeFilteringModel(treeModel);

	setModel(filteringProxyModel);

	treeDelegate = new SceneTreeDelegate();
	setItemDelegate(treeDelegate);

	setDragDropMode(QAbstractItemView::DragDrop);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);
	setContextMenuPolicy(Qt::CustomContextMenu);

	// scene signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Selected(SceneEditor2 *, DAVA::Entity *)), this, SLOT(EntitySelected(SceneEditor2 *, DAVA::Entity *)));
	QObject::connect(SceneSignals::Instance(), SIGNAL(Deselected(SceneEditor2 *, DAVA::Entity *)), this, SLOT(EntityDeselected(SceneEditor2 *, DAVA::Entity *)));

	// particles signals
	QObject::connect(SceneSignals::Instance(), SIGNAL(ParticleLayerValueChanged(SceneEditor2*, DAVA::ParticleLayer*)), this, SLOT(ParticleLayerValueChanged(SceneEditor2*, DAVA::ParticleLayer*)));

	// this widget signals
	QObject::connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeSelectionChanged(const QItemSelection &, const QItemSelection &)));
	QObject::connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(TreeItemClicked(const QModelIndex &)));
	QObject::connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(TreeItemDoubleClicked(const QModelIndex &)));
	QObject::connect(this, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(TreeItemCollapsed(const QModelIndex &)));
	QObject::connect(this, SIGNAL(expanded(const QModelIndex &)), this, SLOT(TreeItemExpanded(const QModelIndex &)));

	QObject::connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenu(const QPoint&)));
}

SceneTree::~SceneTree()
{

}

void SceneTree::SetFilter(const QString &filter)
{
	filteringProxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
}

void SceneTree::GetDropParams(const QPoint &pos, QModelIndex &index, int &row, int &col)
{
	row = -1;
	col = -1;
	index = indexAt(pos);

	switch (dropIndicatorPosition()) 
	{
	case QAbstractItemView::AboveItem:
		row = index.row();
		col = index.column();
		index = index.parent();
		break;
	case QAbstractItemView::BelowItem:
		row = index.row() + 1;
		col = index.column();
		index = index.parent();
		break;
	case QAbstractItemView::OnItem:
	case QAbstractItemView::OnViewport:
		break;
	}
}

void SceneTree::dropEvent(QDropEvent * event)
{
	QTreeView::dropEvent(event);

	if(treeModel->DropAccepted())
	{
		int row, col; 
		QModelIndex parent;

		GetDropParams(event->pos(), parent, row, col);
		expand(parent);
	}

	// after processing don't allow this event to go higher
	// so no body will decide to remove/insert grag&dropped items into treeview
	// except our model. Model will do this when scene entity remove/move signals catched
	event->setDropAction(Qt::IgnoreAction);
}

void SceneTree::dragMoveEvent(QDragMoveEvent *event)
{
	int row, col; 
	QModelIndex parent;

	QTreeView::dragMoveEvent(event);

	GetDropParams(event->pos(), parent, row, col);
	if(!treeModel->DropCanBeAccepted(event->mimeData(), event->dropAction(), row, col, parent))
	{
		event->ignore();
	}
}

void SceneTree::dragEnterEvent(QDragEnterEvent *event)
{
	QTreeView::dragEnterEvent(event);
}

void SceneTree::SceneActivated(SceneEditor2 *scene)
{
	treeModel->SetScene(scene);
	SyncSelectionToTree();
}

void SceneTree::SceneDeactivated(SceneEditor2 *scene)
{
	if(treeModel->GetScene() == scene)
	{
		treeModel->SetScene(NULL);
	}
}

void SceneTree::EntitySelected(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(scene == treeModel->GetScene())
	{
		if(!skipTreeSelectionProcessing)
		{
			skipTreeSelectionProcessing = true;
			SyncSelectionToTree();
			skipTreeSelectionProcessing = false;
		}
	}
}

void SceneTree::EntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity)
{
	if(scene == treeModel->GetScene())
	{
		if(!skipTreeSelectionProcessing)
		{
			skipTreeSelectionProcessing = true;
			SyncSelectionToTree();
			skipTreeSelectionProcessing = false;

		}
	}
}

void SceneTree::TreeSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
	if(!skipTreeSelectionProcessing)
	{
		skipTreeSelectionProcessing = true;
		SyncSelectionFromTree();
		skipTreeSelectionProcessing = false;
	}

	// emit some signal about particles
	EmitParticleSignals(selected);
}

void SceneTree::TreeItemClicked(const QModelIndex & index)
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		// TODO:
		// ...
	}
}

void SceneTree::ParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer)
{
	QModelIndexList indexList = selectionModel()->selection().indexes();
	if (indexList.empty())
	{
		return;
	}

	QModelIndex realIndex = filteringProxyModel->mapToSource(indexList[0]);
	SceneTreeItem *item = treeModel->GetItem(realIndex);
	if (item->ItemType() != SceneTreeItem::EIT_Layer)
	{
		return;
	}

	ParticleLayer* selectedLayer = SceneTreeItemParticleLayer::GetLayer(item);
	if (selectedLayer != layer)
	{
		return;
	}
	
	// Update the "isEnabled" flag, if it is changed.
	bool sceneTreeItemChecked = item->checkState() == Qt::Checked;
	if (layer->GetDisabled() == sceneTreeItemChecked)
	{
		blockSignals(true);
		item->setCheckState(sceneTreeItemChecked ? Qt::Unchecked : Qt::Checked);
		blockSignals(false);
	}
}

void SceneTree::TreeItemDoubleClicked(const QModelIndex & index)
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(filteringProxyModel->mapToSource(index)));
		if(NULL != entity)
		{
			DAVA::AABBox3 box = sceneEditor->selectionSystem->GetSelectionAABox(entity, entity->GetWorldTransform());
			sceneEditor->cameraSystem->LookAt(box);
		}
	}
}

void SceneTree::ShowContextMenu(const QPoint &pos)
{
	QModelIndex index = filteringProxyModel->mapToSource(indexAt(pos));
	SceneTreeItem *item = treeModel->GetItem(index);

	if(NULL != item)
	{
		switch (item->ItemType())
		{
		case SceneTreeItem::EIT_Entity:
			ShowContextMenuEntity(SceneTreeItemEntity::GetEntity(item), mapToGlobal(pos));
			break;
		case SceneTreeItem::EIT_Layer:
			ShowContextMenuLayer(SceneTreeItemParticleLayer::GetLayer(item), mapToGlobal(pos));
			break;
		case SceneTreeItem::EIT_Force:
			ShowContextMenuForce(SceneTreeItemParticleForce::GetForce(item), mapToGlobal(pos));
			break;
		default:
			break;
		}
	}
}

void SceneTree::ShowContextMenuEntity(DAVA::Entity *entity, const QPoint &pos)
{
	if(NULL != entity)
	{
		QMenu contextMenu;

		// look at
		contextMenu.addAction(QIcon(":/QtIcons/zoom.png"), "Look at", this, SLOT(LookAtSelection()));
		contextMenu.addSeparator();

		// add/remove
		contextMenu.addAction(QIcon(":/QtIcons/remove.png"), "Remove entity", this, SLOT(RemoveSelection()));
		contextMenu.addSeparator();

		// lock/unlock
		QAction *lockAction = contextMenu.addAction(QIcon(":/QtIcons/lock_add.png"), "Lock", this, SLOT(LockEntities()));
		QAction *unlockAction = contextMenu.addAction(QIcon(":/QtIcons/lock_delete.png"), "Unlock", this, SLOT(UnlockEntities()));
		if(entity->GetLocked())
		{
			lockAction->setDisabled(true);
		}
		else
		{
			unlockAction->setDisabled(true);
		}

		// custom properties
		DAVA::KeyedArchive *customProp = entity->GetCustomProperties();
		if(NULL != customProp)
		{
			DAVA::FilePath ownerRef = customProp->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
			if(!ownerRef.IsEmpty())
			{
				contextMenu.addSeparator();

				QAction *editModelAction = contextMenu.addAction("Edit Model", this, SLOT(EditModel()));
				QAction *reloadModelAction = contextMenu.addAction("Reload Model", this, SLOT(ReloadModel()));
				QAction *reloadModelAsAction = contextMenu.addAction("Reload Model As...", this, SLOT(ReloadModelAs()));
			}
		}

		// particle effect
		DAVA::ParticleEffectComponent* effect = DAVA::GetEffectComponent(entity);
		if(NULL != effect)
		{
			contextMenu.addSeparator();
			QMenu *particleEffectMenu = contextMenu.addMenu("Particle Effect");
			
			particleEffectMenu->addAction(QIcon(":/QtIcons/emitter_particle.png"), "Add Emitter", this, SLOT(AddEmitter()));
			particleEffectMenu->addSeparator();
			particleEffectMenu->addAction(QIcon(":/QtIcons/play.png"), "Start", this, SLOT(StartEmitter()));
			particleEffectMenu->addAction(QIcon(":/QtIcons/stop.png"), "Stop", this, SLOT(StopEmitter()));
			particleEffectMenu->addAction(QIcon(":/QtIcons/restart.png"), "Restart", this, SLOT(RestartEmitter()));
		}

		// particle emitter
		DAVA::ParticleEmitter* emitter = DAVA::GetEmitter(entity);
		if (NULL != emitter)
		{
			contextMenu.addSeparator();
			QMenu *particleEffectMenu = contextMenu.addMenu("Particle Emitter");
				
			particleEffectMenu->addAction(QIcon(":/QtIcons/layer_particle.png"), "Add Layer", this, SLOT(AddLayer()));
			particleEffectMenu->addSeparator();
			particleEffectMenu->addAction(QIcon(":/QtIcons/openscene.png"), "Load Emitter from Yaml", this, SLOT(LoadEmitterFromYaml()));
			particleEffectMenu->addAction(QIcon(":/QtIcons/savescene.png"), "Save Emitter to Yaml", this, SLOT(SaveEmitterToYaml()));
			particleEffectMenu->addAction(QIcon(":/QtIcons/save_as.png"), "Save Emitter to Yaml As", this, SLOT(SaveEmitterToYamlAs()));
		}

		contextMenu.exec(pos);
	}
}

void SceneTree::ShowContextMenuLayer(DAVA::ParticleLayer *layer, const QPoint &pos)
{

}

void SceneTree::ShowContextMenuForce(DAVA::ParticleForce *force, const QPoint &pos)
{

}

void SceneTree::LookAtSelection()
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup* selection = sceneEditor->selectionSystem->GetSelection();
		if(NULL != selection)
		{
			sceneEditor->cameraSystem->LookAt(selection->GetCommonBbox());
		}
	}
}

void SceneTree::RemoveSelection()
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup* selection = sceneEditor->selectionSystem->GetSelection();
		sceneEditor->structureSystem->Remove(selection);
	}
}

void SceneTree::LockEntities()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			selection->GetEntity(i)->SetLocked(true);
		}
	}
}

void SceneTree::UnlockEntities()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			selection->GetEntity(i)->SetLocked(false);
		}
	}
}

void SceneTree::EditModel()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		int tabIndex = -1;

		for(size_t i = 0; i < selection->Size(); ++i)
		{
			DAVA::Entity *entity = selection->GetEntity(i);
			if(NULL != entity && NULL != entity->GetCustomProperties())
			{
				DAVA::KeyedArchive *archive = entity->GetCustomProperties();
				DAVA::FilePath entityRefPath = archive->GetString(ResourceEditor::EDITOR_REFERENCE_TO_OWNER);
				if(entityRefPath.Exists())
				{
					tabIndex = QtMainWindow::Instance()->GetSceneWidget()->OpenTab(entityRefPath);
				}
			}
		}

		QtMainWindow::Instance()->GetSceneWidget()->SetCurrentTab(tabIndex);
	}
}

void SceneTree::ReloadModel()
{

}

void SceneTree::ReloadModelAs()
{

}

void SceneTree::TreeItemCollapsed(const QModelIndex &index)
{
	treeModel->SetSolid(filteringProxyModel->mapToSource(index), true);

	bool needSync = false;

	// if selected items were inside collapsed item, remove them from selection
	QModelIndexList indexList = selectionModel()->selection().indexes();
	for (int i = 0; i < indexList.size(); ++i)
	{
		QModelIndex childIndex = indexList[i];
		QModelIndex childParent = childIndex.parent();
		while(childParent.isValid())
		{
			if(childParent == index)
			{
				selectionModel()->select(childIndex, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
				needSync = true;
				break;
			}

			childParent = childParent.parent();
		}
	}

	if(needSync)
	{
		SyncSelectionFromTree();
	}
}

void SceneTree::TreeItemExpanded(const QModelIndex &index)
{
	treeModel->SetSolid(filteringProxyModel->mapToSource(index), false);
}

void SceneTree::SyncSelectionToTree()
{
	SceneEditor2* curScene = treeModel->GetScene();
	if(NULL != curScene)
	{
		QModelIndex lastValidIndex;

		selectionModel()->clear();

		const EntityGroup* curSelection = curScene->selectionSystem->GetSelection();
		for(size_t i = 0; i < curSelection->Size(); ++i)
		{
			QModelIndex sIndex = treeModel->GetIndex(curSelection->GetEntity(i));
			sIndex = filteringProxyModel->mapFromSource(sIndex);

			if(sIndex.isValid())
			{
				lastValidIndex = sIndex;
				selectionModel()->select(sIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
			}
		}

		if(lastValidIndex.isValid())
		{
			scrollTo(lastValidIndex, QAbstractItemView::PositionAtCenter);
		}
	}
}

void SceneTree::SyncSelectionFromTree()
{
	SceneEditor2* curScene = treeModel->GetScene();
	if(NULL != curScene)
	{
		QSet<DAVA::Entity*> treeSelectedEntities;

		// select items in scene
		QModelIndexList indexList = selectionModel()->selection().indexes();
		for (int i = 0; i < indexList.size(); ++i)
		{
			DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(filteringProxyModel->mapToSource(indexList[i])));

			if(NULL != entity)
			{
				treeSelectedEntities.insert(entity);
				curScene->selectionSystem->AddSelection(entity);
			}
		}

		// remove from selection system all entities that are not selected in tree
		EntityGroup selGroup = *(curScene->selectionSystem->GetSelection());
		for(size_t i = 0; i < selGroup.Size(); ++i)
		{
			if(!treeSelectedEntities.contains(selGroup.GetEntity(i)))
			{
				curScene->selectionSystem->RemSelection(selGroup.GetEntity(i));
			}
		}
	}
}

void SceneTree::EmitParticleSignals(const QItemSelection & selected)
{
	SceneEditor2* curScene = treeModel->GetScene();
	bool isParticleElements = false;

	// allow only single selected entities
	if(selected.size() == 1) 
	{
		QModelIndexList indexList = selectionModel()->selection().indexes();
		SceneTreeItem *item = treeModel->GetItem(filteringProxyModel->mapToSource(indexList[0]));

		if(NULL != item)
		{
			switch(item->ItemType())
			{
			case SceneTreeItem::EIT_Entity:
				{
					DAVA::Entity *entity = SceneTreeItemEntity::GetEntity(item);
					if(NULL != DAVA::GetEffectComponent(entity))
					{
						SceneSignals::Instance()->EmitEffectSelected(curScene, entity);
						isParticleElements = true;
					}
					else if(NULL != DAVA::GetEmitter(entity))
					{
						SceneSignals::Instance()->EmitEmitterSelected(curScene, entity);
						isParticleElements = true;
					}
				}
				break;
			case SceneTreeItem::EIT_Layer:
				{
					SceneTreeItemParticleLayer *itemLayer = (SceneTreeItemParticleLayer *) item;
					if(NULL != itemLayer->parent && NULL != itemLayer->layer)
					{
						SceneSignals::Instance()->EmitLayerSelected(curScene, itemLayer->layer, false);
						isParticleElements = true;
					}
				}
				break;
			case SceneTreeItem::EIT_Force:
				{
					SceneTreeItemParticleForce *itemForce = (SceneTreeItemParticleForce *) item;
					DAVA::ParticleLayer* layer = itemForce->parent;
					if(NULL != layer)
					{
						for(int i = 0; i < (int) layer->forces.size(); ++i)
						{
							if(layer->forces[i] == itemForce->force)
							{
								SceneSignals::Instance()->EmitForceSelected(curScene, layer, i);
								isParticleElements = true;

								break;
							}
						}
					}
				}
				break;
			}
		}
	}

	if(!isParticleElements)
	{
		SceneSignals::Instance()->EmitEmitterSelected(NULL, NULL);
	}
}

void SceneTree::AddEmitter()
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		QModelIndex realIndex = filteringProxyModel->mapToSource(currentIndex());
		DAVA::Entity *curEntity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(realIndex));
		if(NULL != curEntity && DAVA::GetEffectComponent(curEntity))
		{
			CommandAddParticleEmitter* command = new CommandAddParticleEmitter(curEntity);
			sceneEditor->Exec(command);
			treeModel->ResyncStructure(treeModel->invisibleRootItem(), sceneEditor);
		}
	}
}

void SceneTree::StartEmitter()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			DAVA::ParticleEffectComponent *effect = DAVA::GetEffectComponent(selection->GetEntity(i));
			if(NULL != effect)
			{
				// TODO, Yuri Coder, 2013/07/24. Think about CommandAction's batching.
				CommandStartStopParticleEffect* command = new CommandStartStopParticleEffect(selection->GetEntity(i),
																							 true);
				sceneEditor->Exec(command);
			}
		}
	}
}

void SceneTree::StopEmitter()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			DAVA::ParticleEffectComponent *effect = DAVA::GetEffectComponent(selection->GetEntity(i));
			if(NULL != effect)
			{
				// TODO, Yuri Coder, 2013/07/24. Think about CommandAction's batching.
				CommandStartStopParticleEffect* command = new CommandStartStopParticleEffect(selection->GetEntity(i),
																							 false);
				sceneEditor->Exec(command);
			}
		}
	}
}

void SceneTree::RestartEmitter()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			DAVA::ParticleEffectComponent *effect = DAVA::GetEffectComponent(selection->GetEntity(i));
			if(NULL != effect)
			{
				// TODO, Yuri Coder, 2013/07/24. Think about CommandAction's batching.
				CommandRestartParticleEffect* command = new CommandRestartParticleEffect(selection->GetEntity(i));
				sceneEditor->Exec(command);
			}
		}
	}
}

void SceneTree::AddLayer()
{
	SceneEditor2* sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		QModelIndex realIndex = filteringProxyModel->mapToSource(currentIndex());
		DAVA::Entity *curEntity = SceneTreeItemEntity::GetEntity(treeModel->GetItem(realIndex));
		if(NULL != curEntity && DAVA::GetEmitter(curEntity))
		{
			CommandAddParticleEmitterLayer* command = new CommandAddParticleEmitterLayer(DAVA::GetEmitter(curEntity));
			sceneEditor->Exec(command);
			treeModel->ResyncStructure(treeModel->invisibleRootItem(), sceneEditor);
		}
	}
}

void SceneTree::LoadEmitterFromYaml()
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(NULL != sceneEditor)
	{
		QString filePath = QFileDialog::getOpenFileName(NULL, QString("Open Particle Emitter Yaml file"),
														GetParticlesConfigPath(), QString("YAML File (*.yaml)"));
		if (filePath.isEmpty())
		{
			return;
		}

		const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
		
		bool validationOK = true;
		Set<String> validationErrors;
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			DAVA::ParticleEmitter* emitter = DAVA::GetEmitter(selection->GetEntity(i));
			if(NULL == emitter)
			{
				continue;
			}
			
			// TODO, Yuri Coder, 2013/07/24. Think about CommandAction's batching.
			CommandLoadParticleEmitterFromYaml* command =
				new CommandLoadParticleEmitterFromYaml(emitter, filePath.toStdString());
			sceneEditor->Exec(command);

			// Validate each Particle Emitter after load.
			if (!SceneValidator::Instance()->ValidateParticleEmitter(emitter, validationErrors))
			{
				validationOK = false;
			}
		}

		treeModel->ResyncStructure(treeModel->invisibleRootItem(), sceneEditor);
		
		if (!validationOK)
		{
			ShowErrorDialog(validationErrors);
		}
	}
}

void SceneTree::SaveEmitterToYaml()
{
	PerformSaveEmitter(false);
}

void SceneTree::SaveEmitterToYamlAs()
{
	PerformSaveEmitter(true);
}

void SceneTree::PerformSaveEmitter(bool forceAskFileName)
{
	SceneEditor2 *sceneEditor = treeModel->GetScene();
	if(!sceneEditor)
	{
		return;
	}

	// Verify whether we have to ask about the file name. If at least one emitter
	// does not have emitter path - treat this as "force ask".
	const EntityGroup *selection = sceneEditor->selectionSystem->GetSelection();
	if (forceAskFileName == false)
	{
		for(size_t i = 0; i < selection->Size(); ++i)
		{
			DAVA::ParticleEmitter* emitter = DAVA::GetEmitter(selection->GetEntity(i));
			if (emitter && emitter->GetConfigPath().IsEmpty())
			{
				forceAskFileName = true;
				break;
			}
		}
	}

	FilePath yamlPath;
    if (forceAskFileName)
    {
        QString projectPath = QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
        QString filePath = QFileDialog::getSaveFileName(NULL, QString("Save Particle Emitter YAML file"),
                                                        projectPath, QString("YAML File (*.yaml)"));
		
        if (filePath.isEmpty())
        {
            return;
        }

        yamlPath = FilePath(filePath.toStdString());
	}
	
	// Re-save all the emitters using either YAML path just defined or emitter's
	// inner file path.
	for(size_t i = 0; i < selection->Size(); ++i)
	{
		DAVA::ParticleEmitter* emitter = DAVA::GetEmitter(selection->GetEntity(i));
		if (!emitter)
		{
			continue;
		}

		FilePath curEmitterFilePath;
		if (forceAskFileName)
		{
			curEmitterFilePath = yamlPath;
		}
		else
		{
			curEmitterFilePath = emitter->GetConfigPath().IsEmpty() ? yamlPath : emitter->GetConfigPath();
		}

		CommandSaveParticleEmitterToYaml* command = new CommandSaveParticleEmitterToYaml(emitter, curEmitterFilePath);
		sceneEditor->Exec(command);
	}
}

QString SceneTree::GetParticlesConfigPath()
{
	return QString(EditorSettings::Instance()->GetParticlesConfigsPath().GetAbsolutePathname().c_str());
}
