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

#include "Scene3D/Systems/SkyboxSystem.h"
#include "Render/Highlevel/SkyboxRenderObject.h"
#include "CubemapEditor/CubemapUtils.h"
#include "AddSkyboxDialog.h"

AddSkyboxDialog::AddSkyboxDialog(QWidget* parent)
{
	
}

AddSkyboxDialog::~AddSkyboxDialog()
{
	
}

void AddSkyboxDialog::Show(SceneEditor2* scene)
{
	DVASSERT(scene);
	
	bool skyboxInitiallyPresent = scene->skyboxSystem->IsSkyboxPresent();
	
	Entity* skyboxNode = scene->skyboxSystem->AddSkybox();
	RenderObject* ro = GetRenderObject(skyboxNode);
	
	if(ro &&
	   ro->GetType() == RenderObject::TYPE_SKYBOX)
	{
		SkyboxRenderObject* renderObject = static_cast<SkyboxRenderObject*>(ro);
		
		if(renderObject->GetTextureValidator() == NULL)
		{
			renderObject->SetTextureValidator(new CubemapUtils::CubemapTextureValidator());
		}
		
		FilePath currentTexture = renderObject->GetTexture();
		DAVA::float32 currentOffset = renderObject->GetOffsetZ();
		DAVA::float32 currentRotation = renderObject->GetRotationZ();
		
		BaseAddEntityDialog dlg(this);
		dlg.SetEntity(skyboxNode);
		dlg.setWindowTitle("Set up Skybox");
		dlg.exec();
		
		if(dlg.result() != QDialog::Accepted)
		{
			if(!skyboxInitiallyPresent)
			{
				skyboxNode->GetParent()->RemoveNode(skyboxNode);
				skyboxNode = NULL;
			}
			else
			{
				if(renderObject->GetTexture() != currentTexture)
				{
					renderObject->SetTexture(currentTexture);
				}
				
				if(renderObject->GetRotationZ() != currentRotation)
				{
					renderObject->SetRotationZ(currentRotation);
				}
				
				if(renderObject->GetOffsetZ() != currentOffset)
				{
					renderObject->SetOffsetZ(currentOffset);
				}
			}
		}
		else
		{
			if(!skyboxInitiallyPresent)
			{
				scene->selectionSystem->SetSelection(skyboxNode);
			}
		}
	}
	else
	{
		DVASSERT(false); //skybox must have render object of type RenderObject::TYPE_SKYBOX
	}
}

