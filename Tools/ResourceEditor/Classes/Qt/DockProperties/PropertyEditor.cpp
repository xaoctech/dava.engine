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



#include "DAVAEngine.h"
#include "Scene/SceneDataManager.h"
#include "Entity/Component.h"
#include "Main/mainwindow.h"

#include <QPushButton>

#include "DockProperties/PropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyItem.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataIntrospection.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaVariant.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataDavaKeyedArchive.h"
#include "Commands2/MetaObjModifyCommand.h"
#include "Commands2/InspMemberModifyCommand.h"

#include "PropertyEditorStateHelper.h"

#include "ActionComponentEditor.h"

PropertyEditor::PropertyEditor(QWidget *parent /* = 0 */, bool connectToSceneSignals /*= true*/)
	: QtPropertyEditor(parent)
	, editorMode(EM_NORMAL)
	, curNode(NULL)
	, treeStateHelper(this, curModel)
{
	if(connectToSceneSignals)
	{
		QObject::connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(sceneActivated(SceneEditor2 *)));
		QObject::connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(sceneDeactivated(SceneEditor2 *)));
		QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)), this, SLOT(CommandExecuted(SceneEditor2 *, const Command2*, bool )));
		QObject::connect(SceneSignals::Instance(), SIGNAL(SelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)), this, SLOT(sceneSelectionChanged(SceneEditor2 *, const EntityGroup *, const EntityGroup *)));

		// MainWindow actions
		QObject::connect(QtMainWindow::Instance()->GetUI()->actionShowAdvancedProp, SIGNAL(triggered()), this, SLOT(ActionToggleAdvanced()));
		//advancedMode = QtMainWindow::Instance()->GetUI()->actionShowAdvancedProp->isChecked();
	}
	posSaver.Attach(this, "DocPropetyEditor");

	DAVA::VariantType v = posSaver.LoadValue("splitPos");
	if(v.GetType() == DAVA::VariantType::TYPE_INT32) header()->resizeSection(0, v.AsInt32());

	SetUpdateTimeout(5000);
	SetEditTracking(true);
	SetEditorMode(EM_FAVORITE_EDIT);
}

PropertyEditor::~PropertyEditor()
{
	DAVA::VariantType v(header()->sectionSize(0));
	posSaver.SaveValue("splitPos", v);

	SafeRelease(curNode);
}

void PropertyEditor::SetEntities(const EntityGroup *selected)
{
/*
	DAVA::KeyedArchive *ka = new DAVA::KeyedArchive();
	ResetProperties();
	AppendProperty("test", new QtPropertyDataDavaKeyedArcive(ka));

	return;
*/

    //TODO: support multiselected editing

	SafeRelease(curNode);
    if(NULL != selected && selected->Size() == 1)
 	{
         curNode = SafeRetain(selected->GetEntity(0));
 	}

    ResetProperties();
}

void PropertyEditor::SetEditorMode(eEditoMode mode)
{
	if(editorMode != mode)
	{
		editorMode = mode;
        ResetProperties();
	}
}

void PropertyEditor::ResetProperties()
{
    // Store the current Property Editor Tree state before switching to the new node.
	// Do not clear the current states map - we are using one storage to share opened
	// Property Editor nodes between the different Scene Nodes.
	treeStateHelper.SaveTreeViewState(false);

	RemovePropertyAll();
	if(NULL != curNode)
	{
		// ensure that custom properties exist
		// this call will create them if they are not created yet
		curNode->GetCustomProperties();
        
		// add self introspection info
        AddInsp(QModelIndex(), curNode, curNode->GetTypeInfo());
        
		// add components for this entity
		for(int32 i = 0; i < Component::COMPONENT_COUNT; ++i)
		{
			Component *component = curNode->GetComponent(i);
			if(component)
			{
				AddInsp(QModelIndex(), component, component->GetTypeInfo());
			}
		}
	}
    
	// Restore back the tree view state from the shared storage.
	if (!treeStateHelper.IsTreeStateStorageEmpty())
	{
		treeStateHelper.RestoreTreeViewState();
	}
	else
	{
		// Expand the root elements as default value.
		expandToDepth(0);
	}
}

QModelIndex PropertyEditor::AddInsp(const QModelIndex &parent, void *object, const DAVA::InspInfo *info)
{
	QModelIndex ret;
	QtPropertyData *propData = NULL;

	if(NULL != info)
	{
		bool hasMembers = false;
		const InspInfo *baseInfo = info;

		// check if there are any members in introspection
		while (NULL != baseInfo)
		{
			if(baseInfo->MembersCount() > 0)
			{
				hasMembers = true;
				break;
			}
			baseInfo = baseInfo->BaseInfo();
		}

		// add them
        if(hasMembers)
        {
			propData = new QtPropertyData();
			propData->SetEnabled(false);
			ret = AppendProperty(info->Name(), propData, parent);
			
			OnItemAdded(ret, info->Type());

			while(NULL != baseInfo)
			{
				for(int i = 0; i < baseInfo->MembersCount(); ++i)
				{
					const DAVA::InspMember *member = baseInfo->Member(i);
					AddInspMember(ret, object, member);
				}
				baseInfo = baseInfo->BaseInfo();
			}
		}
    }

	// if this root item was added - colorize it
	if(!parent.isValid() && NULL != propData)
	{
		ApplyStyle(propData, QtPropertyEditor::HEADER_STYLE);
	}

	return ret;
}

QModelIndex PropertyEditor::AddInspMember(const QModelIndex &parent, void *object, const DAVA::InspMember *member)
{
	QModelIndex ret;

	if(NULL != member)
	{
		void *momberObject = member->Data(object);
		const DAVA::InspInfo *memberIntrospection = member->Type()->GetIntrospection(momberObject);

		if(NULL != memberIntrospection && 
			member->Type() != DAVA::MetaInfo::Instance<DAVA::KeyedArchive*>())
		{
			ret = AddInsp(parent, momberObject, memberIntrospection);
		}
		else
		{
			int flags;

			switch(editorMode)
			{
			case EM_NORMAL:
				flags = DAVA::I_EDIT | DAVA::I_VIEW;
				break;
			case EM_ADVANCED:
			case EM_FAVORITE_EDIT:
			case EM_FAVORITE:
				flags = DAVA::I_VIEW;
				break;
			default:
				break;
			}

			QtPropertyData *data = QtPropertyDataIntrospection::CreateMemberData(object, member, flags);
			ret = AppendProperty(member->Name(), data, parent);

			OnItemAdded(ret, member->Type());
		}
	}

	return ret;
}

void PropertyEditor::sceneActivated(SceneEditor2 *scene)
{
	if(NULL != scene)
	{
        const EntityGroup selection = scene->selectionSystem->GetSelection();
		SetEntities(&selection);
	}
}

void PropertyEditor::sceneDeactivated(SceneEditor2 *scene)
{
	SetEntities(NULL);
}

void PropertyEditor::sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
    SetEntities(selected);
}

void PropertyEditor::CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
	int cmdId = command->GetId();

	switch (cmdId)
	{
	case CMDID_COMPONENT_ADD:
	case CMDID_COMPONENT_REMOVE:
	case CMDID_CONVERT_TO_SHADOW:
	case CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML:
		ResetProperties();
		break;
	default:
		Update();
		break;
	}
}

void PropertyEditor::OnItemEdited(const QModelIndex &index)
{
	QtPropertyEditor::OnItemEdited(index);

	QtPropertyData *propData = GetProperty(index);

	if(NULL != propData)
	{
		Command2 *command = (Command2 *) propData->CreateLastCommand();
		if(NULL != command)
		{
			SceneEditor2 *curScene = QtMainWindow::Instance()->GetCurrentScene();
			if(NULL != curScene)
			{
				curScene->Exec(command);
			}
		}
	}
}

void PropertyEditor::OnItemAdded(const QModelIndex &index, const DAVA::MetaInfo *itemMeta)
{
	if(index.isValid() && NULL != itemMeta)
	{
		QtPropertyData *propData = GetProperty(index);

		if(NULL != propData)
		{
			if(DAVA::MetaInfo::Instance<DAVA::ActionComponent>() == itemMeta)
			{
				// Add optional button to edit action component
				QPushButton *editActions = new QPushButton(QIcon(":/QtIcons/settings.png"), "");
				editActions->setFlat(true);

				propData->AddOW(QtPropertyOW(editActions, true));
				QObject::connect(editActions, SIGNAL(pressed()), this, SLOT(ActionEditComponent()));
			}
			else if(DAVA::MetaInfo::Instance<DAVA::RenderObject>() == itemMeta)
			{
				// Add optional button to bake transform render object
				QPushButton *bakeButton = new QPushButton(QIcon(":/QtIcons/transform_bake.png"), "");
				bakeButton->setToolTip("Bake Transform");
				bakeButton->setIconSize(QSize(12, 12));

				propData->AddOW(QtPropertyOW(bakeButton));
				QObject::connect(bakeButton, SIGNAL(pressed()), this, SLOT(ActionBakeTransform()));
			}
		}
	}
}

void PropertyEditor::mouseReleaseEvent(QMouseEvent *event)
{
	bool skipEvent = false;
	QModelIndex index = indexAt(event->pos());

	if(index.parent().isValid() && index.column() == 0)
	{
		QRect rect = visualRect(index);
		rect.setX(0);
		rect.setWidth(16);

		if(rect.contains(event->pos()))
		{
			QtPropertyData *data = GetProperty(index);
			if(NULL != data && !IsParentFavorite(data))
			{
				SetFavorite(data, !IsFavorite(data));
				skipEvent = true;
			}
		}
	}

	if(!skipEvent)
	{
		QtPropertyEditor::mouseReleaseEvent(event);
	}
}

void PropertyEditor::drawRow(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	static QIcon favIcon = QIcon(":/QtIcons/star.png");
	static QIcon nfavIcon = QIcon(":/QtIcons/star_empty.png");

	QStyleOptionViewItemV4 opt = option;
	if(index.parent().isValid() && editorMode == EM_FAVORITE_EDIT)
	{
		QtPropertyData *data = GetProperty(index);
		if(NULL != data)
		{
			if(!IsParentFavorite(data))
			{
				if(IsFavorite(data))
				{
					favIcon.paint(painter, opt.rect.x(), opt.rect.y(), 16, opt.rect.height());
				}
				else
				{
					nfavIcon.paint(painter, opt.rect.x(), opt.rect.y(), 16, opt.rect.height());
				}
			}
		}
	}

	QtPropertyEditor::drawRow(painter, opt, index);
}

void PropertyEditor::ActionToggleAdvanced()
{
	QAction *showAdvancedAction = dynamic_cast<QAction *>(QObject::sender());
	if(NULL != showAdvancedAction)
	{
		// TODO:
		// toggle

		//SetEditorMode(EM_ADVANCED);
		//SetAdvancedMode(showAdvancedAction->isChecked());
	}
}

void PropertyEditor::ActionEditComponent()
{
	if(NULL != curNode)
	{
		ActionComponentEditor editor;

		editor.SetComponent((DAVA::ActionComponent*)curNode->GetComponent(DAVA::Component::ACTION_COMPONENT));
		editor.exec();
	}	
}

void PropertyEditor::ActionBakeTransform()
{
	if(NULL != curNode)
	{
		DAVA::RenderObject * ro = GetRenderObject(curNode);
		if(NULL != ro)
		{
			ro->BakeTransform(curNode->GetLocalTransform());
			curNode->SetLocalTransform(DAVA::Matrix4::IDENTITY);
		}
	}
}

bool PropertyEditor::IsFavorite(QtPropertyData *data) const
{
	return (data->GetUserData().toBool() || scheme.contains(data->GetPath()));
}

bool PropertyEditor::IsParentFavorite(QtPropertyData *data) const
{
	bool ret = false;

	QtPropertyData *parent = data->Parent();
	while(NULL != parent)
	{
		if(IsFavorite(parent))
		{
			ret = true;
			break;
		}
		else
		{
			parent = parent->Parent();
		}
	}

	return ret;
}

void PropertyEditor::SetFavorite(QtPropertyData *data, bool favorite)
{
	data->SetUserData(favorite);
	if(favorite)
	{
		scheme.insert(data->GetPath());
	}
	else
	{
		scheme.remove(data->GetPath());
	}
}
