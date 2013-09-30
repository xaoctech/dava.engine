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

#include "ui_BaseAddEntityDialog.h"

#include "Scene3D/Systems/SkyboxSystem.h"
#include "Render/Highlevel/SkyboxRenderObject.h"
#include "CubemapEditor/CubemapUtils.h"
#include "AddSkyboxDialog.h"

AddSkyboxDialog::AddSkyboxDialog(QWidget* parent) : BaseAddEntityDialog(parent)
{
	controlButton = new QPushButton();
	closeHandled = false;
	editorScene = NULL;
	initialState.initialSkyboxNode = NULL;
	
	ui->lowerLayOut->removeWidget(ui->buttonBox);
	ui->lowerLayOut->addWidget(controlButton, 0, 0);
	ui->lowerLayOut->addWidget(ui->buttonBox, 0, 1);
	
	QObject::connect(this, SIGNAL(finished(int)), this, SLOT(OnFinished(int)));
}

AddSkyboxDialog::~AddSkyboxDialog()
{
	QObject::disconnect(this, SIGNAL(finished(int)), this, SLOT(OnFinished(int)));
	
	if(!closeHandled)
	{
		OnFinished(QDialog::Rejected);
	}
	
	SafeRelease(initialState.initialSkyboxNode);
	
	delete controlButton;
	SafeRelease(editorScene);
}

void AddSkyboxDialog::OnFinished(int code)
{
	closeHandled = true;
	
	if(code != QDialog::Accepted)
	{
		Entity* currentSkybox = editorScene->skyboxSystem->GetSkybox();
		
		if(NULL == initialState.initialSkyboxNode)
		{
			if(currentSkybox)
			{
				currentSkybox->GetParent()->RemoveNode(currentSkybox);
			}
		}
		else
		{
			if(NULL == currentSkybox)
			{
				currentSkybox = editorScene->skyboxSystem->AddSkybox();
			}
			
			SkyboxRenderObject* curRenderObj = static_cast<SkyboxRenderObject*>(GetRenderObject(currentSkybox));
			
			curRenderObj->ForceSetOffsetZ(initialState.offset);
			curRenderObj->SetRotationZ(initialState.rotation);
			curRenderObj->SetTexture(initialState.texture);
			if(curRenderObj->GetTextureValidator() == NULL)
			{
				curRenderObj->SetTextureValidator(new CubemapUtils::CubemapTextureValidator());
			}
		}
	}
}

void AddSkyboxDialog::OnCreateButtonClicked()
{
	if(editorScene)
	{
		Entity* skyboxNode = editorScene->skyboxSystem->AddSkybox();
		RenderObject* ro = GetRenderObject(skyboxNode);
		
		if(ro &&
		   ro->GetType() == RenderObject::TYPE_SKYBOX)
		{
			SkyboxRenderObject* renderObject = static_cast<SkyboxRenderObject*>(ro);
			
			if(renderObject->GetTextureValidator() == NULL)
			{
				renderObject->SetTextureValidator(new CubemapUtils::CubemapTextureValidator());
			}			
		}
		
		MakeDeleteButton();
		UpdateEntity(skyboxNode);
	}
	else
	{
		DVASSERT(false);
	}
}

void  AddSkyboxDialog::OnDeleteButtonClicked()
{
	Entity* skyboxEntity = GetEntity();
		
	if(skyboxEntity)
	{
		skyboxEntity->GetParent()->RemoveNode(skyboxEntity);
		UpdateEntity(NULL);
		
		MakeCreateButton();
	}
}

void AddSkyboxDialog::MakeCreateButton()
{
	QObject::disconnect(controlButton, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));
	QObject::connect(controlButton, SIGNAL(clicked()), this, SLOT(OnCreateButtonClicked()));
	controlButton->setText("Create");	
}

void AddSkyboxDialog::MakeDeleteButton()
{
	QObject::connect(controlButton, SIGNAL(clicked()), this, SLOT(OnDeleteButtonClicked()));
	QObject::disconnect(controlButton, SIGNAL(clicked()), this, SLOT(OnCreateButtonClicked()));
	controlButton->setText("Delete");	
}

void AddSkyboxDialog::UpdateEntity(Entity* newEntity)
{
	SafeRelease(entity);
	
	entity = newEntity;
	SafeRetain(entity);

	PropertyEditor* pEditor = ui->propEditor;
	
	pEditor->SetNode(entity);
	
	if(entity)
	{
		pEditor->expandAll();
		PerformResize();
	}
}

void AddSkyboxDialog::Show(QWidget* parent, SceneEditor2* scene)
{
	DVASSERT(scene);
	
	Entity* currentSkybox = scene->skyboxSystem->GetSkybox();
	
	AddSkyboxDialog* dlg = new AddSkyboxDialog(parent);
	dlg->setAttribute(Qt::WA_DeleteOnClose, true);
	dlg->SetEditorScene(scene);
	dlg->UpdateEntity(currentSkybox);
	dlg->SetInitialState(currentSkybox);
	dlg->setWindowTitle("Set up Skybox");
	dlg->show();
}

void AddSkyboxDialog::SetEditorScene(SceneEditor2* scene)
{
	SafeRelease(editorScene);
	editorScene = SafeRetain(scene);
}

SceneEditor2* AddSkyboxDialog::GetEditorScene() const
{
	return editorScene;
}

void AddSkyboxDialog::SetInitialState(Entity* skyboxState)
{
	SafeRelease(initialState.initialSkyboxNode);
	initialState.initialSkyboxNode = SafeRetain(skyboxState);
	
	if(initialState.initialSkyboxNode)
	{
		SkyboxRenderObject* curRenderObj = static_cast<SkyboxRenderObject*>(GetRenderObject(initialState.initialSkyboxNode));
		
		DVASSERT(curRenderObj);
		initialState.offset = curRenderObj->GetOffsetZ();
		initialState.rotation = curRenderObj->GetRotationZ();
		initialState.texture = curRenderObj->GetTexture();
		
		MakeDeleteButton();
	}
	else
	{
		MakeCreateButton();
	}
}

