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

namespace DAVA
{
    
/*NMaterial * MaterialSystem::GetMaterial(const FastName & name)
{
    NMaterial * material = materials.GetValue(name);
    if (!material)
    {
        material = new NMaterial();
        bool result = material->LoadFromFile(name.c_str());
        //materials.Insert(name, material);
    }
    return material;
}*/
	
NMaterial * MaterialSystem::GetMaterial(const FastName & name)
{
	NMaterial* material = materials.GetValue(name);
	
	DVASSERT(material);
	
	return material;
}
	
void MaterialSystem::BuildMaterialList(NMaterial* parent, /*out*/ Vector<NMaterial*>& materialList)
{
	if(NULL == parent)
	{
		HashMap<FastName, NMaterial*>::Iterator mapIter = materials.Begin();
		while(mapIter != materials.End())
		{
			NMaterial* material = mapIter.GetValue();
			if(NULL == material->state.parent)
			{
				materialList.push_back(material);
				BuildMaterialList(material, materialList);
			}
			
			++mapIter;
		}
	}
	else
	{
		for(int i = 0; i < parent->children.size(); ++i)
		{
			NMaterial* material = parent->children[i];
			materialList.push_back(material);
			BuildMaterialList(material, materialList);
		}
	}
}

bool MaterialSystem::LoadMaterialConfig(const FilePath& filePath)
{
	YamlParser * parser = YamlParser::Create(filePath);
    if (!parser)
    {
        Logger::Error("[MaterialSystem::LoadMaterialConfig] Can't load material tree configuration: %s", filePath.GetAbsolutePathname().c_str());
        return false;
    }
	
    YamlNode * rootNode = parser->GetRootNode();
    
    if (!rootNode)
    {
        SafeRelease(rootNode);
		SafeRelease(parser);
        return false;
    }

	Map<String, Vector<MaterialData> > nodes;
	int32 nodeCount = rootNode->GetCount();
	for(int32 i = 0; i < nodeCount; ++i)
	{
		const YamlNode* materialNode = rootNode->Get(i);
		if(materialNode->AsString() == "Material")
		{
			MaterialData data;
			
			const YamlNode* nameNode = materialNode->Get("Name");
			DVASSERT(nameNode);
			
			if(nameNode)
			{
				data.name = nameNode->AsString();
				
				DVASSERT(data.name.size() > 0);
				if(data.name.size() > 0)
				{
					const YamlNode* pathNode = materialNode->Get("Path");
					DVASSERT(pathNode);
					
					if(pathNode)
					{
						data.path = pathNode->AsString();
						
						DVASSERT(data.path.size() > 0);
						if(data.path.size() > 0)
						{
							//only parent node is optional
							const YamlNode* parentNode = materialNode->Get("Parent");
							if(parentNode)
							{
								data.parent = parentNode->AsString();
							}
							
							Map<String, Vector<MaterialData> >::iterator iter = nodes.find(data.parent);
							if(iter != nodes.end())
							{
								Vector<MaterialData>& dataList = iter->second;
								dataList.push_back(data);
							}
							else
							{
								Vector<MaterialData> dataList;
								dataList.push_back(data);
								nodes[data.parent] = dataList;
							}
						}
						else
						{
							Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %s in the material configuration has empty Path. Skipping...", data.name.c_str());
						}
					}
					else
					{
						Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %s in the material configuration doesn't contain Path. Skipping...", data.name.c_str());
					}
				}
				else
				{
					Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %d in the material configuration has empty Name. Skipping...", i);
				}
			}
			else
			{
				Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %d in the material configuration doesn't contain Name. Skipping...", i);
			}
		}
	}
	
	SafeRelease(rootNode);
	SafeRelease(parser);
	
	//TODO: validate material tree structure. It shouldn't contain loops or nodes belonging to several roots
	
	//fetch root nodes
	Map<String, Vector<MaterialData> >::iterator rootsIter = nodes.find("");
	DVASSERT(rootsIter != nodes.end());
	if(rootsIter != nodes.end())
	{
		Vector<MaterialData>& roots = rootsIter->second;
		size_t rootCount = roots.size();
		for(size_t i = 0; i < rootCount; ++i)
		{
			MaterialData& currentData = roots[i];
			LoadMaterial(currentData.name, currentData.path, NULL, nodes);
		}
	}
	else
	{
		Logger::Error("[MaterialSystem::LoadMaterialConfig] Material config %s contains no root material(s). Skipping...", filePath.GetAbsolutePathname().c_str());
		return false;
	}
	
	return true;
}
	
NMaterial* MaterialSystem::LoadMaterial(const FastName& name,
										const FilePath& filePath,
										NMaterial* parentMaterial,
										Map<String, Vector<MaterialData> >& nodes)
{
	NMaterial* material = new NMaterial();
	bool result = material->LoadFromFile(filePath.GetAbsolutePathname());
	
	DVASSERT(result);
	if(result)
	{
		material->SetName(name.c_str());
		material->SetMaterialName(name.c_str());
		materials.Insert(name, material);
		material->SetParent(parentMaterial);
		
		Map<String, Vector<MaterialData> >::iterator childrenIter = nodes.find(material->GetName());
		if(childrenIter != nodes.end())
		{
			Vector<MaterialData>& materials = childrenIter->second;
			size_t materialCount = materials.size();
			for(size_t i = 0; i < materialCount; ++i)
			{
				MaterialData& currentData = materials[i];
				LoadMaterial(currentData.name, currentData.path, material, nodes);
			}
		}
	}
	else
	{
		Logger::Error("[MaterialSystem::LoadMaterialConfig] Failed to load material %s", filePath.GetAbsolutePathname().c_str());
		SafeRelease(material);
	}
	
	return material;
}

};

