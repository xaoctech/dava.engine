/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Material/MaterialSystem.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"
#include "Render/Material/NMaterial.h"
#include "Render/3D/PolygonGroup.h"
#include "FileSystem/FileSystem.h"
#include "Render/Shader.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"
#include "Render/RenderManager.h"
#include "Render/Material/NMaterialTemplate.h"

namespace DAVA
{
	const FastName MaterialSystem::DEFAULT_QUALITY_NAME = FastName("Normal");
		
	MaterialSystem::MaterialSystem()
	{
	}
	
	MaterialSystem::~MaterialSystem()
	{
	}
	
	void MaterialSystem::BuildMaterialList(Vector<NMaterial*>& materialList) const
	{
	}
	
	void MaterialSystem::BuildMaterialList(const FastName& materialName, Vector<NMaterial*>& materialList) const
	{
	}
		
	void MaterialSystem::BuildMaterialList(NMaterial::eMaterialType materialType, Vector<NMaterial*>& materialList) const
	{
	}
	
	void MaterialSystem::SetDefaultMaterialQuality(const FastName& qualityLevelName)
	{
		defaultMaterialQuality = qualityLevelName;
	}
	
	const FastName& MaterialSystem::GetDefaultMaterialQuality() const
	{
		return defaultMaterialQuality;
	}
	
	const FastName& MaterialSystem::GetCurrentMaterialQuality() const
	{
		return currentMaterialQuality;
	}
	
	void MaterialSystem::SwitchMaterialQuality(const FastName& qualityLevelName)
	{
	}
	
	//VI: creates material of type MATERIALTYPE_INSTANCE
	//VI: These methods DO NOT add newly created materials to the material system
	NMaterial* MaterialSystem::CreateMaterialInstance()
	{
		static int32 instanceCounter = 0;
		instanceCounter++;
		
		NMaterial* mat = new NMaterial();
		mat->SetMaterialType(NMaterial::MATERIALTYPE_INSTANCE);
		mat->SetMaterialKey((NMaterial::NMaterialKey)mat);
		mat->SetMaterialName(Format("Instance-%d", instanceCounter));
		mat->SetName(mat->GetMaterialName().c_str());
		
		return mat;
	}
	
	//VI: creates material of type MATERIALTYPE_MATERIAL
	//VI: These methods DO NOT add newly created materials to the material system
	NMaterial* MaterialSystem::CreateMaterial(const FastName& materialName,
											  const FastName& templateName,
											  const FastName& defaultQuality)
	{
		NMaterial* mat = new NMaterial();
		mat->SetMaterialType(NMaterial::MATERIALTYPE_MATERIAL);
		mat->SetMaterialKey((NMaterial::NMaterialKey)mat); //this value may be temporary
		mat->SetMaterialName(materialName.c_str());
		mat->SetName(mat->GetMaterialName().c_str());
		
		const NMaterialTemplate* matTemplate = NMaterialTemplateCache::Instance()->Get(templateName);
		DVASSERT(matTemplate);
		mat->SetMaterialTemplate(matTemplate, defaultQuality);
		
		return mat;
	}
};

