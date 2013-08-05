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

#include "Scene3D/Entity.h"
#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Systems/SkyboxSystem.h"
#include "Scene3D/Components/SkyboxComponent.h"

namespace DAVA
{
	
	SkyboxComponent::SkyboxComponent() : verticalOffset(0), rotationAngle(0)
	{
		
	}
	
	SkyboxComponent::~SkyboxComponent()
	{
		
	}
	
	Component* SkyboxComponent::Clone(Entity * toEntity)
	{
		SkyboxComponent* newSkybox = new SkyboxComponent();
		newSkybox->SetEntity(GetEntity());
		newSkybox->SetTexture(texturePath);
		newSkybox->SetVerticalOffset(verticalOffset);
		newSkybox->SetRotationAngle(rotationAngle);
		
		return newSkybox;
	}
	
	void SkyboxComponent::Serialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
	{
		Component::Serialize(archive, sceneFile);
		
		if(archive != NULL)
		{
			archive->SetString("sbc.texture", texturePath.GetRelativePathname());
			archive->SetFloat("sbc.verticalOffset", verticalOffset);
			archive->SetFloat("sbc.rotation", rotationAngle);
		}
	}
	
	void SkyboxComponent::Deserialize(KeyedArchive *archive, SceneFileV2 *sceneFile)
	{
		Component::Deserialize(archive, sceneFile);
		
		if(archive != NULL)
		{
			texturePath = archive->GetString("sbc.texture");
			verticalOffset = archive->GetFloat("sbc.verticalOffset");
			rotationAngle = archive->GetFloat("sbc.rotation");
		}
	}
	
	void SkyboxComponent::SetTexture(const FilePath& texPath)
	{
		texturePath = texPath;
		
		if(GetEntity() != NULL &&
		   GetEntity()->GetScene() != NULL)
		{
			GetEntity()->GetScene()->skyboxSystem->SetSystemStateDirty();
		}
	}
	
	FilePath SkyboxComponent::GetTexture()
	{
		return texturePath;
	}
	
	void SkyboxComponent::SetVerticalOffset(const float32& offset)
	{
		verticalOffset = offset;
		
		if(GetEntity() != NULL &&
		   GetEntity()->GetScene() != NULL)
		{
			GetEntity()->GetScene()->skyboxSystem->SetSystemStateDirty();
		}
	}
	
	float32 SkyboxComponent::GetVerticalOffset()
	{
		return verticalOffset;
	}
	
	void SkyboxComponent::SetRotationAngle(const float32& rotation)
	{
		rotationAngle = rotation;
		
		if(GetEntity() != NULL &&
		   GetEntity()->GetScene() != NULL)
		{
			GetEntity()->GetScene()->skyboxSystem->SetSystemStateDirty();
		}
	}
	
	float32 SkyboxComponent::GetRotationAngle()
	{
		return rotationAngle;
	}
	
	Vector3 SkyboxComponent::GetBoxSize()
	{
		return Vector3(1.0f, 1.0f, 1.0f);
	}
}
