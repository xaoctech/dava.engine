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
#ifndef __DAVAENGINE_MATERIAL_SYSTEM_H__
#define __DAVAENGINE_MATERIAL_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Scene3D/DataNode.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/NMaterial.h"

namespace DAVA
{
    
class MaterialGraph;
class MaterialGraphNode;
class NMaterial;
class PolygonGroup;

class MaterialSystem
{
public:
	
	static FastName GLOBAL_NAME;
	static NMaterial::NMaterialKey GLOBAL_KEY;
	
public:
	
	MaterialSystem();
	~MaterialSystem();
	
	//VI: material names can be duplicate. Returns first material found
    NMaterial* GetMaterial(const FastName& name);
	NMaterial* GetMaterial(NMaterial::NMaterialKey materialKey);
	
	void BuildMaterialList(Vector<NMaterial*>& materialList) const;
	void BuildMaterialList(const FastName& materialName, Vector<NMaterial*>& materialList) const;
	void BuildMaterialList(NMaterial::eMaterialType materialType, Vector<NMaterial*>& materialList) const;
	
	void SetDefaultMaterialQuality(const FastName& qualityLevelName);
	const FastName& GetDefaultMaterialQuality() const;
	const FastName& GetCurrentMaterialQuality() const;
	void SwitchMaterialQuality(const FastName& qualityLevelName);
	
	static NMaterial* CreateMaterialInstance();
		
	static NMaterial* CreateMaterial(const FastName& materialName,
									 const FastName& templateName);
		
private:
	
	FastName currentMaterialQuality;
	FastName defaultMaterialQuality;
};

};

#endif // __DAVAENGINE_MATERIAL_SYSTEM_H__

