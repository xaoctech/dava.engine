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



#include "AddSwitchEntityDialog.h"
#include "Tools/MimeDataHelper/MimeDataHelper.h"
#include "Tools/SelectPathWidget/SelectEntityPathWidget.h"
#include "Main/mainwindow.h"
#include "SceneEditor/EditorSettings.h"
#include <QLabel>
#include "Classes/Commands2/EntityAddCommand.h"
#include "ui_BaseAddEntityDialog.h"


AddSwitchEntityDialog::AddSwitchEntityDialog( QWidget* parent)
		:BaseAddEntityDialog(parent)
{
	setAcceptDrops(true);
	setAttribute( Qt::WA_DeleteOnClose, true );
	FilePath defaultPath(EditorSettings::Instance()->GetProjectPath().GetAbsolutePathname() + "/DataSource/3d");
	
	SceneEditor2 *scene = QtMainWindow::Instance()->GetCurrentScene();
	if(scene)
	{
		FilePath scenePath = scene->GetScenePath();
		if(scenePath.Exists())
		{
			defaultPath = scenePath.GetDirectory();
		}
	}
	
	SelectEntityPathWidget* firstWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(),"");
	SelectEntityPathWidget* secondWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(),"");
	SelectEntityPathWidget* thirdWidget = new SelectEntityPathWidget(parent, defaultPath.GetAbsolutePathname(),"");

	AddControlToUserContainer(firstWidget, "First Entity:");
	AddControlToUserContainer(secondWidget, "Second Entity:");
	AddControlToUserContainer(thirdWidget, "Third Entity:");

	pathWidgets.push_back(firstWidget);
	pathWidgets.push_back(secondWidget);
	pathWidgets.push_back(thirdWidget);

	Entity* entityToAdd = new Entity();
	entityToAdd->SetName(ResourceEditor::SWITCH_NODE_NAME);
	entityToAdd->AddComponent(ScopedPtr<SwitchComponent> (new SwitchComponent()));
	KeyedArchive *customProperties = entityToAdd->GetCustomProperties();
	customProperties->SetBool(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
	SetEntity(entityToAdd);
	SafeRelease(entityToAdd);
}

AddSwitchEntityDialog::~AddSwitchEntityDialog()
{
	RemoveAllControlsFromUserContainer();
	Q_FOREACH(SelectEntityPathWidget* widget, pathWidgets)
	{
		delete widget;
	}
}

void AddSwitchEntityDialog::CleanupPathWidgets()
{
	Q_FOREACH(SelectEntityPathWidget* widget, pathWidgets)
	{
		widget->EraseWidget();
	}
}

void AddSwitchEntityDialog::GetPathEntities(DAVA::Vector<DAVA::Entity*>& entities, SceneEditor2* editor)
{
	Q_FOREACH(SelectEntityPathWidget* widget, pathWidgets)
	{
		Entity* entity = widget->GetOutputEntity(editor);
		if(entity)
		{
			entities.push_back(entity);
		}
	}
}

void AddSwitchEntityDialog::accept()
{
	SceneEditor2* scene = QtMainWindow::Instance()->GetCurrentScene();
	
	Entity* switchEntity = entity;
	
	if( NULL == scene)
	{
		CleanupPathWidgets();
		return;
	}
	
	Vector<Entity*> vector;
	GetPathEntities(vector, scene);
	CleanupPathWidgets();
	
	Q_FOREACH(Entity* item, vector)
	{
		if(item)
		{
			Entity *e = item->Clone();
			switchEntity->AddNode(e);
			e->Release();
		}
	}
	if(vector.size() > 0)
	{
		EntityAddCommand* command = new EntityAddCommand(switchEntity, scene);
		scene->Exec(command);
		
		Entity* affectedEntity = command->GetEntity();
		scene->selectionSystem->SetSelection(affectedEntity);
        GlobalEventSystem::Instance()->Event(affectedEntity,EventSystem::SWITCH_CHANGED);
    }
	
	BaseAddEntityDialog::accept();
}

void AddSwitchEntityDialog::reject()
{
	CleanupPathWidgets();
	BaseAddEntityDialog::reject();
}
