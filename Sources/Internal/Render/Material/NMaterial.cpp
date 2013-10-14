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


#include "Render/Material/MaterialSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/ShaderCache.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
    
    
	NMaterialDescriptor::NMaterialDescriptor()
	{
		
	}
    
	NMaterialDescriptor::~NMaterialDescriptor()
	{
		
	}
	
	uint32 NMaterialDescriptor::GetTextureSlotByName(const String & textureName)
	{
		Map<String, uint32>::iterator it = slotNameMap.find(textureName);
		if (it != slotNameMap.end())
		{
			return it->second;
		}
		// if we haven't found slot let's use first slot
		return 0;
	}
    
	uint32 NMaterialDescriptor::GetUniformSlotByName(const String & uniformName)
	{
		Map<String, uint32>::iterator it = uniformNameMap.find(uniformName);
		if (it != uniformNameMap.end())
		{
			return it->second;
		}
		// if we haven't found slot let's use first slot
		return 0;
	}
    
	void NMaterialDescriptor::SetNameForTextureSlot(uint32 slot, const String & name)
	{
		slotNameMap[name] = slot;
	}
	
	void NMaterialDescriptor::SetNameForUniformSlot(uint32 slot, const String & name)
	{
		uniformNameMap[name] = slot;
	}
	
	MaterialTechnique::MaterialTechnique(const FastName & _shaderName, const FastNameSet & _uniqueDefines, RenderState * _renderState)
	{
		shader = 0;
		shaderName = _shaderName;
		uniqueDefines = _uniqueDefines;
		renderState = _renderState;
	}
    
	MaterialTechnique::~MaterialTechnique()
	{
		SafeRelease(shader);
	}
	
	void MaterialTechnique::RecompileShader(const FastNameSet& materialDefines)
	{
		FastNameSet combinedDefines = materialDefines;
		
		if(uniqueDefines.Size() > 0)
		{
			combinedDefines.Combine(uniqueDefines);
		}
		
		SafeRelease(shader);
		shader = SafeRetain(ShaderCache::Instance()->Get(shaderName, combinedDefines));
	}
    
	const FastName NMaterial::TEXTURE_ALBEDO("albedo");
	const FastName NMaterial::TEXTURE_NORMAL("normal");
	const FastName NMaterial::TEXTURE_DETAIL("detail");
	const FastName NMaterial::TEXTURE_LIGHTMAP("lightmap");
	const FastName NMaterial::TEXTURE_DECAL("decal");
	
	static FastName TEXTURE_NAME_PROPS[] = {
		NMaterial::TEXTURE_ALBEDO,
		NMaterial::TEXTURE_NORMAL,
		NMaterial::TEXTURE_DETAIL,
		NMaterial::TEXTURE_LIGHTMAP,
		NMaterial::TEXTURE_DECAL
	};
	
	NMaterialState::NMaterialState() :
	layers(4),
	techniqueForRenderPass(8),
	nativeDefines(16),
	materialProperties(32),
	textures(8)
	{
		parent = NULL;
	}
	
	NMaterialState::~NMaterialState()
	{
		for(size_t i = 0; i < children.size(); ++i)
		{
			children[i]->SetParent(NULL);
		}
		
		for(HashMap<FastName, TextureBucket*>::Iterator i = textures.Begin();
			i != textures.End();
			++i)
		{
			SafeRelease(i.GetValue()->texture);
			delete i.GetValue();
		}
		
		textures.Clear();
		texturesArray.clear();
	}
	
	void NMaterialState::AddMaterialProperty(const String & keyName, const YamlNode * uniformNode)
	{
		FastName uniformName = keyName;
		Logger::Debug("Uniform Add:%s %s", keyName.c_str(), uniformNode->AsString().c_str());
		
		Shader::eUniformType type = Shader::UT_FLOAT;
		union
		{
			float val;
			float valArray[4];
			float valMatrix[4 * 4];
			int32 valInt;
		}data;
		
		uint32 size = 0;
		
		if (uniformNode->GetType() == YamlNode::TYPE_STRING)
		{
			String uniformValue = uniformNode->AsString();
			if (uniformValue.find('.') != String::npos)
				type = Shader::UT_FLOAT;
			else
				type = Shader::UT_INT;
		}
		else if (uniformNode->GetType() == YamlNode::TYPE_ARRAY)
		{
			size = uniformNode->GetCount();
			uint32 arrayCount = 0;
			for (uint32 k = 0; k < size; ++k)
			{
				if (uniformNode->Get(k)->GetType() == YamlNode::TYPE_ARRAY)
				{
					arrayCount++;
				}
			}
			if (size == arrayCount)
			{
				if (size == 2)type = Shader::UT_FLOAT_MAT2;
				else if (size == 3)type = Shader::UT_FLOAT_MAT3;
				else if (size == 4)type = Shader::UT_FLOAT_MAT4;
			}else if (arrayCount == 0)
			{
				if (size == 2)type = Shader::UT_FLOAT_VEC2;
				else if (size == 3)type = Shader::UT_FLOAT_VEC3;
				else if (size == 4)type = Shader::UT_FLOAT_VEC4;
			}else
			{
				DVASSERT(0 && "Something went wrong");
			}
		}
		
		switch (type) {
			case Shader::UT_INT:
				data.valInt = uniformNode->AsInt();
				break;
			case Shader::UT_FLOAT:
				data.val = uniformNode->AsFloat();
				break;
			case Shader::UT_FLOAT_VEC2:
			case Shader::UT_FLOAT_VEC3:
			case Shader::UT_FLOAT_VEC4:
				for (uint32 k = 0; k < size; ++k)
				{
					data.valArray[k] = uniformNode->Get(k)->AsFloat();
				}
				break;
				
			default:
				Logger::Error("Wrong material property or format not supported.");
				break;
		}
		
		NMaterialProperty * materialProperty = new NMaterialProperty();
		materialProperty->size = 1;
		materialProperty->type = type;
		materialProperty->data = new char[Shader::GetUniformTypeSize(type)];
		
		memcpy(materialProperty->data, &data, Shader::GetUniformTypeSize(type));
		
		materialProperties.Insert(uniformName, materialProperty);
	}
    
	void NMaterialState::SetPropertyValue(const FastName & propertyFastName, Shader::eUniformType type, uint32 size, const void * data)
	{
		NMaterialProperty * materialProperty = materialProperties.GetValue(propertyFastName);
		if (materialProperty)
		{
			if (materialProperty->type != type || materialProperty->size != size)
			{
				char* tmpPtr = static_cast<char*>(materialProperty->data);
				SafeDeleteArray(tmpPtr); //cannot delete void* pointer
				materialProperty->size = size;
				materialProperty->type = type;
				materialProperty->data = new char[Shader::GetUniformTypeSize(type) * size];
			}
		}
		else
		{
			materialProperty = new NMaterialProperty;
			materialProperty->size = size;
			materialProperty->type = type;
			materialProperty->data = new char[Shader::GetUniformTypeSize(type) * size];
			materialProperties.Insert(propertyFastName, materialProperty);
		}
		
		memcpy(materialProperty->data, data, size * Shader::GetUniformTypeSize(type));
	}
	
	NMaterialProperty* NMaterialState::GetMaterialProperty(const FastName & keyName)
	{
		NMaterialState * currentMaterial = this;
		NMaterialProperty * property = NULL;
		while(currentMaterial != 0)
		{
			property = currentMaterial->materialProperties.GetValue(keyName);
			if (property)
			{
				break;
			}
			
			currentMaterial = currentMaterial->parent;
		}
		
		return property;
	}
	
	void NMaterialState::SetMaterialName(const String& name)
	{
		materialName = name;
	}
	
	FastName NMaterialState::GetMaterialName()
	{
		return materialName;
	}
	
	FastName NMaterialState::GetParentName()
	{
		return parentName;
	}

	void NMaterialState::AddMaterialTechnique(const FastName & techniqueName, MaterialTechnique * materialTechnique)
	{
		techniqueForRenderPass.Insert(techniqueName, materialTechnique);
	}
    
	MaterialTechnique * NMaterialState::GetTechnique(const FastName & techniqueName)
	{
		MaterialTechnique * technique = techniqueForRenderPass.GetValue(techniqueName);

		DVASSERT(technique != 0);
		return technique;
	}
    
    
	void NMaterialState::SetTexture(const FastName & textureFastName, Texture * texture)
	{
		TextureBucket* bucket = textures.GetValue(textureFastName);
		if(NULL != bucket)
		{
			DVASSERT(bucket->index < texturesArray.size());
			
			if(bucket->texture != texture)
			{
				SafeRelease(bucket->texture);
				bucket->texture = SafeRetain(texture);
				texturesArray[bucket->index] = texture;
			}
		}
		else
		{
			TextureBucket* bucket = new TextureBucket();
			bucket->texture = SafeRetain(texture);
			bucket->index = texturesArray.size();
			
			textures.Insert(textureFastName, bucket);
			texturesArray.push_back(texture);
			textureNamesArray.push_back(textureFastName);
			textureSlotArray.push_back(-1);
		}
	}
    
	Texture * NMaterialState::GetTexture(const FastName & textureFastName) const
	{
		TextureBucket* bucket = textures.GetValue(textureFastName);
		if (!bucket)
		{
			NMaterial * currentMaterial = parent;
			while(currentMaterial != 0)
			{
				bucket = currentMaterial->textures.GetValue(textureFastName);
				if (bucket)
				{
					// TODO: Find effective way to store parent techniques in children, to avoid cycling through them
					// As a first decision we can use just store of this technique in child material map.
					break;
				}
				currentMaterial = currentMaterial->parent;
			}
		}
		return (bucket != NULL) ? bucket->texture : NULL;
	}
    
	Texture * NMaterialState::GetTexture(uint32 index)
	{
		return texturesArray[index];
	}
	
	uint32 NMaterialState::GetTextureCount()
	{
		return (uint32)texturesArray.size();
	}
	
	void NMaterialState::SetParentToState(NMaterial* material)
	{
		parent = SafeRetain(material);
		parentName = (NULL == parent) ? "" : parent->GetMaterialName();
	}
	
	void NMaterialState::AddChildToState(NMaterial* material)
	{
		SafeRetain(material);
		children.push_back(material);
	}
	
	void NMaterialState::RemoveChildFromState(NMaterial* material)
	{
		Vector<NMaterial*>::iterator child = std::find(children.begin(), children.end(), material);
		if(children.end() != child)
		{
			children.erase(child);
		}
	}
	
	void NMaterialState::AddMaterialDefineToState(const FastName& defineName)
	{
		nativeDefines.Insert(defineName);
	}
	
	void NMaterialState::RemoveMaterialDefineFromState(const FastName& defineName)
	{
		nativeDefines.Remove(defineName);
	}
	
	void NMaterialState::ShallowCopyTo(NMaterialState* targetState)
	{
		DVASSERT(this != targetState);
		
		targetState->nativeDefines.Clear();
		targetState->layers.Clear();
		targetState->textures.Clear();
		targetState->texturesArray.clear();
		targetState->textureNamesArray.clear();
		targetState->textureSlotArray.clear();

		targetState->parentName = parentName;
		targetState->materialName = materialName;
		
		targetState->layers.Combine(layers);
		
		targetState->nativeDefines.Combine(nativeDefines);
		
		targetState->techniqueForRenderPass.Clear();
		for(HashMap<FastName, MaterialTechnique *>::Iterator it = techniqueForRenderPass.Begin();
			it != techniqueForRenderPass.End();
			++it)
		{
			targetState->techniqueForRenderPass.Insert(it.GetKey(), it.GetValue());
		}
		
		targetState->materialProperties.Clear();
		for(HashMap<FastName, NMaterialProperty *>::Iterator it = materialProperties.Begin();
			it != materialProperties.End();
			++it)
		{
			targetState->materialProperties.Insert(it.GetKey(), it.GetValue());
		}

		
		for(HashMap<FastName, TextureBucket *>::Iterator it = textures.Begin();
			it != textures.End();
			++it)
		{
			targetState->SetTexture(it.GetKey(), it.GetValue()->texture);
		}
	}
	
	bool NMaterialState::LoadFromYamlNode(const YamlNode* stateNode)
	{
		bool result = false;
		
		const YamlNode * parentNameNode = stateNode->Get("Parent");
		if (parentNameNode)
		{
			parentName = parentNameNode->AsString();
		}

		
		const YamlNode * layersNode = stateNode->Get("Layers");
		if (layersNode)
		{
			int32 count = layersNode->GetCount();
			for (int32 k = 0; k < count; ++k)
			{
				const YamlNode * singleLayerNode = layersNode->Get(k);
				layers.Insert(FastName(singleLayerNode->AsString()));
			}
		}
		
		const YamlNode * uniformsNode = stateNode->Get("Uniforms");
		if (uniformsNode)
		{
			uint32 count = uniformsNode->GetCount();
			for (uint32 k = 0; k < count; ++k)
			{
				const YamlNode * uniformNode = uniformsNode->Get(k);
				if (uniformNode)
				{
					AddMaterialProperty(uniformsNode->GetItemKeyName(k), uniformNode);
				}
			}
		}
		
		const YamlNode * materialDefinesNode = stateNode->Get("MaterialDefines");
		if (materialDefinesNode)
		{
			uint32 count = materialDefinesNode->GetCount();
			for (uint32 k = 0; k < count; ++k)
			{
				const YamlNode * defineNode = materialDefinesNode->Get(k);
				if (defineNode)
				{
					nativeDefines.Insert(FastName(defineNode->AsString().c_str()));
				}
			}
		}
		
		uint32 techniqueCount = 0;
		for (int32 k = 0; k < stateNode->GetCount(); ++k)
		{
			const YamlNode * renderStepNode = stateNode->Get(k);
			
			if (renderStepNode->AsString() == "RenderPass")
			{
				Logger::Debug("- RenderPass found: %s", renderStepNode->AsString().c_str());
				const YamlNode * shaderNode = renderStepNode->Get("Shader");
				const YamlNode * shaderGraphNode = renderStepNode->Get("ShaderGraph");
				
				if (!shaderNode && !shaderGraphNode)
				{
					Logger::Error("RenderPass:%s does not have shader or shader graph", renderStepNode->AsString().c_str());
					break;
				}
				
				FastName shaderName;
				if (shaderNode)
				{
					shaderName = FastName(shaderNode->AsString().c_str());
				}
				
				if (shaderGraphNode)
				{
					String shaderGraphPathname = shaderGraphNode->AsString();
					MaterialGraph * graph = new MaterialGraph();
					graph->LoadFromFile(shaderGraphPathname);
					
					MaterialCompiler * compiler = new MaterialCompiler();
					compiler->Compile(graph, 0, 4, 0);
										
					SafeRelease(compiler);
					SafeRelease(graph);
				}
				
				FastNameSet definesSet;
				const YamlNode * definesNode = renderStepNode->Get("UniqueDefines");
				if (definesNode)
				{
					int32 count = definesNode->GetCount();
					for (int32 k = 0; k < count; ++k)
					{
						const YamlNode * singleDefineNode = definesNode->Get(k);
						definesSet.Insert(FastName(singleDefineNode->AsString().c_str()));
					}
				}
				
				RenderState * renderState = new RenderState();
				//const YamlNode * renderStateNode = renderStepNode->Get("RenderState");
				if (renderStepNode)
				{
					renderState->LoadFromYamlNode(renderStepNode);
				}
				
				const YamlNode * renderPassNameNode = renderStepNode->Get("Name");
				FastName renderPassName;
				if (renderPassNameNode)
				{
					renderPassName = renderPassNameNode->AsString();
				}
				
				MaterialTechnique * technique = new MaterialTechnique(shaderName, definesSet, renderState);
				AddMaterialTechnique(renderPassName, technique);
				
				techniqueCount++;
			}
		}
				
		result = true;
		
		return result;
	}
	
	void NMaterialState::DeepCopyTo(NMaterialState* targetState)
	{
		DVASSERT(this != targetState);
		
		targetState->nativeDefines.Clear();
		targetState->layers.Clear();
		targetState->textures.Clear();
		targetState->texturesArray.clear();
		targetState->textureNamesArray.clear();
		targetState->textureSlotArray.clear();

        if(parentName.IsValid()) targetState->parentName = parentName;
		if(materialName.IsValid()) targetState->materialName = materialName;
		targetState->layers.Combine(layers);
		targetState->nativeDefines.Combine(nativeDefines);
		
		for(HashMap<FastName, TextureBucket *>::Iterator it = textures.Begin();
			it != textures.End();
			++it)
		{
			targetState->SetTexture(it.GetKey(), it.GetValue()->texture);
		}
		
		for(Vector<NMaterial*>::iterator it = children.begin();
			it != children.end();
			++it)
		{
			targetState->children.push_back(SafeRetain(*it));
		}
		
		HashMap<FastName, MaterialTechnique *>::Iterator techIter = techniqueForRenderPass.Begin();
		while(techIter != techniqueForRenderPass.End())
		{
			MaterialTechnique* technique = techIter.GetValue();
			
			RenderState* newRenderState = new RenderState();
			technique->GetRenderState()->CopyTo(newRenderState);
			MaterialTechnique* childMaterialTechnique = new MaterialTechnique(technique->GetShaderName(),
																			  technique->GetUniqueDefineSet(),
																			  newRenderState);
			targetState->AddMaterialTechnique(techIter.GetKey(), childMaterialTechnique);
			
			++techIter;
		}
		
		HashMap<FastName, NMaterialProperty*>::Iterator propIter = materialProperties.Begin();
		while(propIter != materialProperties.End())
		{
			targetState->materialProperties.Insert(propIter.GetKey(), propIter.GetValue()->Clone());
			
			++propIter;
		}
	}
	
	NMaterialState* NMaterialState::CloneState()
	{
		NMaterialState* clonedState = new NMaterialState();
				
		DeepCopyTo(clonedState);
		
		return clonedState;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	uint64 NMaterial::uniqueIdSequence = 0;
	
	NMaterial::NMaterial() : NMaterialState(),
	inheritedDefines(16),
	effectiveLayers(8),
	states(4)
	{
		activeTechnique = 0;
		ready = false;
		materialDynamicLit = false;
		configMaterial = false;
	}
    
	NMaterial::~NMaterial()
	{
		//VI: TODO: It would be really great to remove itself from the material system here
		//VI: but it's not really good to store pointer to material system in each material
		
		SetParent(NULL);
	}
    
	bool NMaterial::LoadFromFile(const String & pathname)
	{
		bool result = false;
		YamlParser * parser = YamlParser::Create(pathname);
		if (!parser)
		{
			Logger::Error("Can't load requested material: %s", pathname.c_str());
			return result;
		}
		
		YamlNode * rootNode = parser->GetRootNode();
		
		if (!rootNode)
		{
			SafeRelease(rootNode);
			SafeRelease(parser);
			return result;
		}
		
		const YamlNode * materialNode = rootNode->Get("Material");
		
		uint32 materialStateCount = 0;
		for(uint32 i = 0; i < materialNode->GetCount(); ++i)
		{
			const YamlNode* materialStateNode = materialNode->Get(i);
			
			if (materialStateNode->AsString() == "MaterialState")
			{
				materialStateCount++;
			}
		}
		
		for(uint32 i = 0; i < materialNode->GetCount(); ++i)
		{
			const YamlNode* materialStateNode = materialNode->Get(i);
			
			if (materialStateNode->AsString() == "MaterialState")
			{
				const YamlNode* materialStateNameNode = materialStateNode->Get("Name");
				
				if(materialStateNameNode)
				{
					String materialStateName = materialStateNameNode->AsString();
					
					if(!states.IsKey(materialStateName))
					{
						if(materialStateCount > 1)
						{
							NMaterialState* materialState = new NMaterialState();
							if(materialState->LoadFromYamlNode(materialStateNode))
							{
								states.Insert(materialStateName, materialState);
							}
							else
							{
								Logger::Error("[NMaterial::LoadFromFile] Failed to load a material state %s in file %s!",
											  materialStateName.c_str(),
											  pathname.c_str());
								
								DVASSERT(false);
							}
						}
						else
						{
							LoadFromYamlNode(materialStateNode);
							currentStateName = materialStateName;
						}
					}
					else
					{
						Logger::Error("[NMaterial::LoadFromFile] Duplicate material state %s found in file %s!",
									  materialStateName.c_str(),
									  pathname.c_str());
						
						DVASSERT(false);
					}
					
				}
				else
				{
					Logger::Error("[NMaterial::LoadFromFile] There's a material state without a name in file %s!", pathname.c_str());
					DVASSERT(false);
				}
			}
		}
				
		result = true;
		SafeRelease(parser);
		return result;
	}
	
	void NMaterial::BindMaterialTechnique(const FastName & techniqueName, Camera* camera)
	{
		if(!ready)
		{
			Rebuild(false);
		}
		
		if (techniqueName != activeTechniqueName)
		{
			activeTechnique = GetTechnique(techniqueName);
			if (activeTechnique)
				activeTechniqueName = techniqueName;
		}
		
		SetupPerFrameProperties(camera);
		
		RenderState* renderState = activeTechnique->GetRenderState();
		
		NMaterial* texMat = this;
		while(texMat)
		{
			BindTextures(texMat, renderState);
			texMat = texMat->parent;
		}
		
		Shader * shader = activeTechnique->GetShader();
		renderState->SetShader(shader);
		
		RenderManager::Instance()->FlushState(renderState);
		
		uint32 uniformCount = shader->GetUniformCount();
		for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			if (uniform->id == Shader::UNIFORM_NONE)
			{
				NMaterial * currentMaterial = this;
				
				while(currentMaterial != 0)
				{
					NMaterialProperty * property = currentMaterial->materialProperties.GetValue(uniform->name);
					if (property)
					{
						shader->SetUniformValueByIndex(uniformIndex, uniform->type, uniform->size, property->data);
						break;
					}
					currentMaterial = currentMaterial->parent;
				}
			}
		}
	}
	
	void NMaterial::BindTextures(NMaterial* curMaterial, RenderState* rs)
	{
		size_t textureCount = curMaterial->texturesArray.size();
		for(size_t i = 0; i < textureCount; ++i)
		{
			int32 textureSlot = curMaterial->textureSlotArray[i];
			if(textureSlot < 0)
			{
				NMaterialProperty* prop = curMaterial->GetMaterialProperty(curMaterial->textureNamesArray[i]);
				DVASSERT(prop);
				
				textureSlot = *(int32*)prop->data;
				curMaterial->textureSlotArray[i] = textureSlot;
			}
			
			rs->SetTexture(curMaterial->texturesArray[i], textureSlot);
		}
	}
	
	void NMaterial::Draw(PolygonGroup * polygonGroup)
	{
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
		RenderManager::Instance()->AttachRenderData();
		
		// TODO: rethink this code
		if (polygonGroup->renderDataObject->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, 0);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, polygonGroup->indexArray);
		}
	}
	
	void NMaterial::Draw(RenderDataObject*	renderData, uint16* indices, uint16 indexCount)
	{
		DVASSERT(renderData);
		
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(renderData);
		RenderManager::Instance()->AttachRenderData();
		
		// TODO: rethink this code
		if (renderData->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, EIF_16, 0);
		}
		else
		{
			if(renderData->indexCount)
			{
				RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, EIF_16, renderData->indices);
			}
			else
			{
				RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, indexCount, EIF_16, indices);
			}
		}
	}
    
	void NMaterial::SetParent(NMaterial* material)
	{
		if(parent)
		{
			parent->RemoveChild(this);
		}
		
		ResetParent();
		SetParentToState(material);
		
		if(parent)
		{
			parent->AddChild(this);
		}
	}
	
	void NMaterial::AddChild(NMaterial* material)
	{
		DVASSERT(material);
		DVASSERT(material != this);
		//sanity check if such material is already present
		DVASSERT(std::find(NMaterialState::children.begin(), NMaterialState::children.end(), material) == NMaterialState::children.end());
		
		if(material)
		{
			AddChildToState(material);
			
			//TODO: propagate properties such as defines, textures, render state etc here
			material->OnParentChanged();
		}
	}
	
	void NMaterial::RemoveChild(NMaterial* material)
	{
		DVASSERT(material);
		DVASSERT(material != this);
		
		Vector<NMaterial*>::iterator child = std::find(NMaterialState::children.begin(), NMaterialState::children.end(), material);
		if(NMaterialState::children.end() != child)
		{
			material->ResetParent();
			
			RemoveChildFromState(material);
		}
	}
	
	void NMaterial::ResetParent()
	{
		//TODO: clear parent states such as textures, defines, etc
		effectiveLayers.Clear();
		effectiveLayers.Combine(layers);
		UnPropagateParentDefines();
		
		//SafeRelease(state.parent);
		SetParentToState(NULL);
	}
	
	void NMaterial::PropagateParentDefines()
	{
		ready = false;
		inheritedDefines.Clear();
		if(parent)
		{
			if(parent->inheritedDefines.Size() > 0)
			{
				inheritedDefines.Combine(parent->inheritedDefines);
			}
			
			if(parent->nativeDefines.Size() > 0)
			{
				inheritedDefines.Combine(parent->nativeDefines);
			}
		}
	}
	
	void NMaterial::UnPropagateParentDefines()
	{
		ready = false;
		inheritedDefines.Clear();
	}
	
	void NMaterial::Rebuild(bool recursive)
	{
		FastNameSet combinedDefines = inheritedDefines;
		
		if(nativeDefines.Size() > 0)
		{
			combinedDefines.Combine(nativeDefines);
		}
		
		HashMap<FastName, MaterialTechnique *>::Iterator iter = techniqueForRenderPass.Begin();
		while(iter != techniqueForRenderPass.End())
		{
			MaterialTechnique* technique = iter.GetValue();
			technique->RecompileShader(combinedDefines);
			
			Shader* shader = technique->GetShader();
			
			//VI: assume that uniform "lightPosition0" indicates vertex or pixel lighting
			int lightPositionUniformIndex = shader->FindUniformIndexByName("lightPosition0");
			materialDynamicLit = (lightPositionUniformIndex >= 0);
			++iter;
		}
		
		if(recursive)
		{
			size_t childrenCount = NMaterialState::children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				NMaterialState::children[i]->Rebuild(recursive);
			}
		}
		
		ready = true;
	}
	
	void NMaterial::AddMaterialDefine(const FastName& defineName)
	{
		ready = false;
		AddMaterialDefineToState(defineName);
		
		NotifyChildrenOnChange();
	}
	
	void NMaterial::RemoveMaterialDefine(const FastName& defineName)
	{
		ready = false;
		RemoveMaterialDefineFromState(defineName);
		
		NotifyChildrenOnChange();
	}
	
	const FastNameSet& NMaterial::GetRenderLayers()
	{
		return effectiveLayers;
	}
	
	void NMaterial::OnParentChanged()
	{
		PropagateParentLayers();
		PropagateParentDefines();
		
		NotifyChildrenOnChange();
	}
	
	void NMaterial::NotifyChildrenOnChange()
	{
		size_t count = NMaterialState::children.size();
		for(size_t i = 0; i < count; ++i)
		{
			NMaterialState::children[i]->OnParentChanged();
		}
	}
	
	void NMaterial::PropagateParentLayers()
	{
		effectiveLayers.Clear();
		effectiveLayers.Combine(layers);
		
		if(parent)
		{
			const FastNameSet& parentLayers = parent->GetRenderLayers();
			effectiveLayers.Combine(parentLayers);
		}
	}
	
	NMaterial* NMaterial::CreateChild()
	{
		String childName = Format("%s.%lu", materialName.c_str(), uniqueIdSequence++);
		
		NMaterial* childMaterial = NULL;
		
		if(!IsSwitchable())
		{
			childMaterial = new NMaterial();
			HashMap<FastName, MaterialTechnique *>::Iterator iter = techniqueForRenderPass.Begin();
			while(iter != techniqueForRenderPass.End())
			{
				MaterialTechnique* technique = iter.GetValue();
				
				RenderState* newRenderState = new RenderState();
				technique->GetRenderState()->CopyTo(newRenderState);
				MaterialTechnique* childMaterialTechnique = new MaterialTechnique(technique->GetShaderName(),
																				  technique->GetUniqueDefineSet(),
																				  newRenderState);
				childMaterial->AddMaterialTechnique(iter.GetKey(), childMaterialTechnique);
				
				++iter;
			}
		}
		else
		{
			childMaterial = Clone();
		}
        
        childMaterial->SetMaterialName(childName);
		
		return childMaterial;
	}
	
	void NMaterial::SetupPerFrameProperties(Camera* camera)
	{
		//VI: this is vertex or pixel lit material
		//VI: setup light for the material
		//VI: TODO: deal with multiple lights
		if(camera && materialDynamicLit && lights[0])
		{
			NMaterialProperty* propAmbientColor = GetMaterialProperty("prop_ambientColor");
			NMaterialProperty* propDiffuseColor = GetMaterialProperty("prop_diffuseColor");
			NMaterialProperty* propSpecularColor = GetMaterialProperty("prop_specularColor");
			
			const Matrix4 & matrix = camera->GetMatrix();
			Vector3 lightPosition0InCameraSpace = lights[0]->GetPosition() * matrix;
			Color materialAmbientColor = (propAmbientColor) ? *(Color*)propAmbientColor->data : Color(1, 1, 1, 1);
			Color materialDiffuseColor = (propDiffuseColor) ? *(Color*)propDiffuseColor->data : Color(1, 1, 1, 1);
			Color materialSpecularColor = (propSpecularColor) ? *(Color*)propSpecularColor->data : Color(1, 1, 1, 1);
			float32 intensity = lights[0]->GetIntensity();
			
			materialAmbientColor = materialAmbientColor * lights[0]->GetAmbientColor();
			materialDiffuseColor = materialDiffuseColor * lights[0]->GetDiffuseColor();
			materialSpecularColor = materialSpecularColor * lights[0]->GetSpecularColor();
			
			SetPropertyValue("lightPosition0", Shader::UT_FLOAT_VEC3, 1, lightPosition0InCameraSpace.data);
			
			SetPropertyValue("materialLightAmbientColor", Shader::UT_FLOAT_VEC3, 1, &materialAmbientColor);
			SetPropertyValue("materialLightDiffuseColor", Shader::UT_FLOAT_VEC3, 1, &materialDiffuseColor);
			SetPropertyValue("materialLightSpecularColor", Shader::UT_FLOAT_VEC3, 1, &materialSpecularColor);
			SetPropertyValue("lightIntensity0", Shader::UT_FLOAT, 1, &intensity);
		}
	}
	
	void NMaterial::Save(KeyedArchive * archive, SerializationContext * serializationContext)
	{
		if(!IsSwitchable())
		{
			KeyedArchive* defaultStateArchive = new KeyedArchive();
			Serialize(*this, defaultStateArchive, serializationContext);
			archive->SetArchive("__defaultState__", defaultStateArchive);
			SafeRelease(defaultStateArchive);
		}
		else
		{
			HashMap<FastName, NMaterialState*>::Iterator stateIter = states.Begin();
			while(stateIter != states.End())
			{
				KeyedArchive* stateArchive = new KeyedArchive();
				Serialize(*stateIter.GetValue(), stateArchive, serializationContext);
				archive->SetArchive(stateIter.GetKey().c_str(), stateArchive);
				SafeRelease(stateArchive);
				
				++stateIter;
			}
		}
	}
	
	void NMaterial::Load(KeyedArchive * archive, SerializationContext * serializationContext)
	{
		//TODO: add code allowing to transition from switchable to non-switchable materials and vice versa
		const Map<String, VariantType*>& archiveData = archive->GetArchieveData();
		
		if(archive->Count() > 1)
		{
			for(Map<String, VariantType*>::const_iterator it = archiveData.begin();
				it != archiveData.end();
				++it)
			{
				if(VariantType::TYPE_KEYED_ARCHIVE == it->second->type)
				{
					NMaterialState* matState = new NMaterialState();
					Deserialize(*matState, it->second->AsKeyedArchive(), serializationContext);
					states.Insert(it->first, matState);
				}
			}
			
			SetMaterialName(states.Begin().GetValue()->GetMaterialName().c_str());
		}
		else
		{
			KeyedArchive* stateArchive = archive->GetArchive("__defaultState__");
			Deserialize(*this, stateArchive, serializationContext);
		}
	}
	
	void NMaterial::Serialize(const NMaterialState& materialState,
							  KeyedArchive * archive,
							  SerializationContext * serializationContext)
	{
		archive->SetString("materialName", materialState.materialName.c_str());
		archive->SetString("parentName", (materialState.parentName.IsValid()) ? materialState.parentName.c_str() : "");
		
		KeyedArchive* materialLayers = new KeyedArchive();
		SerializeFastNameSet(materialState.layers, materialLayers);
		archive->SetArchive("layers", materialLayers);
		SafeRelease(materialLayers);
		
		KeyedArchive* materialNativeDefines = new KeyedArchive();
		SerializeFastNameSet(materialState.nativeDefines, materialNativeDefines);
		archive->SetArchive("nativeDefines", materialNativeDefines);
		SafeRelease(materialNativeDefines);
		
		KeyedArchive* materialProps = new KeyedArchive();
		for(HashMap<FastName, NMaterialProperty*>::Iterator it = materialState.materialProperties.Begin();
			it != materialState.materialProperties.End();
			++it)
		{
			NMaterialProperty* property = it.GetValue();
			
			uint32 propDataSize = property->GetDataSize();
			uint8* propertyStorage = new uint8[propDataSize + sizeof(uint32) + sizeof(uint32)];
			
			uint32 uniformType = property->type; //make sure uniform type is always uint32
			memcpy(propertyStorage, &uniformType, sizeof(uint32));
			memcpy(propertyStorage + sizeof(uint32), &property->size, sizeof(uint32));
			memcpy(propertyStorage + sizeof(uint32) + sizeof(uint32), property->data, propDataSize);
			
			materialProps->SetByteArray(it.GetKey().c_str(), propertyStorage, propDataSize + sizeof(uint32) + sizeof(uint32));
			
			SafeDeleteArray(propertyStorage);
		}
		archive->SetArchive("properties", materialProps);
		SafeRelease(materialProps);
		
		KeyedArchive* materialTextures = new KeyedArchive();
		for(HashMap<FastName, TextureBucket*>::Iterator it = materialState.textures.Begin();
			it != materialState.textures.End();
			++it)
		{
			materialTextures->SetString(it.GetKey().c_str(), it.GetValue()->texture->GetPathname().GetRelativePathname(serializationContext->GetScenePath()));
		}
		archive->SetArchive("textures", materialTextures);
		SafeRelease(materialTextures);
		
		int techniqueIndex = 0;
		KeyedArchive* materialTechniques = new KeyedArchive();
		for(HashMap<FastName, MaterialTechnique *>::Iterator it = materialState.techniqueForRenderPass.Begin();
			it != materialState.techniqueForRenderPass.End();
			++it)
		{
			MaterialTechnique* technique = it.GetValue();
			KeyedArchive* techniqueArchive = new KeyedArchive();
			
			techniqueArchive->SetString("renderPass", it.GetKey().c_str());
			techniqueArchive->SetString("shaderName", it.GetValue()->GetShaderName().c_str());
			
			KeyedArchive* techniqueDefines = new KeyedArchive();
			const FastNameSet& techniqueDefinesSet = technique->GetUniqueDefineSet();
			SerializeFastNameSet(techniqueDefinesSet, techniqueDefines);
			techniqueArchive->SetArchive("defines", techniqueDefines);
			SafeRelease(techniqueDefines);
			
			KeyedArchive* techniqueRenderState = new KeyedArchive();
			RenderState* renderState = technique->GetRenderState();
			renderState->Serialize(techniqueRenderState, serializationContext);
			techniqueArchive->SetArchive("renderState", techniqueRenderState);
			SafeRelease(techniqueRenderState);
			
			materialTechniques->SetArchive(Format("technique.%d", techniqueIndex), techniqueArchive);
			SafeRelease(techniqueArchive);
			techniqueIndex++;
		}
		archive->SetArchive("techniques", materialTechniques);
		SafeRelease(materialTechniques);
	}
	
	void NMaterial::Deserialize(NMaterialState& materialState,
								KeyedArchive * archive,
								SerializationContext * serializationContext)
	{
		materialState.parentName = archive->GetString("parentName");
		materialState.materialName = archive->GetString("materialName");
		
		DeserializeFastNameSet(archive->GetArchive("layers"), materialState.layers);
		DeserializeFastNameSet(archive->GetArchive("nativeDefines"), materialState.nativeDefines);
		
		const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = propsMap.begin();
			it != propsMap.end();
			++it)
		{
			const VariantType* propVariant = it->second;
			DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
			DVASSERT(propVariant->AsByteArraySize() >= (sizeof(uint32) + sizeof(uint32)));
			
			const uint8* ptr = propVariant->AsByteArray();
			
			materialState.SetPropertyValue(it->first, (Shader::eUniformType)*(const uint32*)ptr, *(((const uint32*)ptr) + 1), ptr + sizeof(uint32) + sizeof(uint32));
		}
		
		const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = texturesMap.begin();
			it != texturesMap.end();
			++it)
		{
			Texture* tex = NULL;
			String relativePathname = it->second->AsString();
			
			if (!relativePathname.empty())
			{
				FilePath texturePath;
				if(relativePathname[0] == '~') //path like ~res:/Gfx...
				{
					texturePath = FilePath(relativePathname);
				}
				else
				{
					texturePath = serializationContext->GetScenePath() + relativePathname;
				}
				
				if(serializationContext->IsDebugLogEnabled())
					Logger::Debug("--- load material texture: %s src:%s", relativePathname.c_str(), texturePath.GetAbsolutePathname().c_str());
				
				tex = Texture::CreateFromFile(texturePath);
			}
			
			materialState.SetTexture(it->first, tex);
		}
		
		const Map<String, VariantType*>& techniquesMap = archive->GetArchive("techniques")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = techniquesMap.begin();
			it != techniquesMap.end();
			++it)
		{
			KeyedArchive* techniqueArchive = it->second->AsKeyedArchive();
			
			String renderPassName = techniqueArchive->GetString("renderPass");
			String shaderName = techniqueArchive->GetString("shaderName");
			
			FastNameSet techniqueDefines;
			DeserializeFastNameSet(techniqueArchive->GetArchive("defines"), techniqueDefines);
			
			KeyedArchive* renderStateArchive = techniqueArchive->GetArchive("renderState");
			RenderState* renderState = new RenderState();
			renderState->Deserialize(renderStateArchive, serializationContext);
			
			MaterialTechnique* technique = new MaterialTechnique(shaderName, techniqueDefines, renderState);
			materialState.AddMaterialTechnique(renderPassName, technique);
		}		
	}
	
	void NMaterial::DeserializeFastNameSet(const KeyedArchive* srcArchive, FastNameSet& targetSet)
	{
		const Map<String, VariantType*>& setData = srcArchive->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = setData.begin();
			it != setData.end();
			++it)
		{
			targetSet.Insert(it->first);
		}
	}
	
	void NMaterial::SerializeFastNameSet(const FastNameSet& srcSet, KeyedArchive* targetArchive)
	{
		for(FastNameSet::Iterator it = srcSet.Begin();
			it != srcSet.End();
			++it)
		{
			targetArchive->SetBool(it.GetKey().c_str(), true);
		}
	}
	
	bool NMaterial::SwitchState(const FastName& stateName,
								MaterialSystem* materialSystem,
								bool forceSwitch)
	{
		if(!forceSwitch && (stateName == currentStateName))
		{
			return true;
		}
		
		NMaterialState* state = states.GetValue(stateName);
		
		if(state != NULL)
		{
			SetParent(NULL);
			
			inheritedDefines.Clear();
			effectiveLayers.Clear();
			
			//{TODO: these should be removed and changed to a generic system
			//setting properties via special setters
			lightCount = 0;
			materialDynamicLit = false;
			//}END TODO
			
			activeTechniqueName.Reset();
			activeTechnique = NULL;
			ready = false;

			if(currentStateName.IsValid() && !forceSwitch)
			{
				NMaterialState* prevState = states.GetValue(currentStateName);
				DVASSERT(prevState);
				
				ShallowCopyTo(prevState);
			}
			
			state->ShallowCopyTo(this);
			
			NMaterial* newParent = materialSystem->GetMaterial(parentName);
			DVASSERT(newParent);
			if(NULL == newParent)
			{
				newParent = materialSystem->GetDefaultMaterial();
			}
			
			SetParent(newParent);
			
			currentStateName = stateName;
			
			if(techniqueForRenderPass.Size() == 0)
			{
				//VI: try to copy renderpasses from corresponding parent material when there's no own renderpasses
				HashMap<FastName, MaterialTechnique *>::Iterator iter = newParent->techniqueForRenderPass.Begin();
				while(iter != newParent->techniqueForRenderPass.End())
				{
					MaterialTechnique* technique = iter.GetValue();
					
					RenderState* newRenderState = new RenderState();
					technique->GetRenderState()->CopyTo(newRenderState);
					MaterialTechnique* materialTechnique = new MaterialTechnique(technique->GetShaderName(),
																					  technique->GetUniqueDefineSet(),
																					  newRenderState);
					AddMaterialTechnique(iter.GetKey(), materialTechnique);
					
					++iter;
				}

			}
		}
		
		return (state != NULL);
	}
	
	bool NMaterial::IsSwitchable() const
	{
		return (states.Size() > 0);
	}
	
	NMaterial* NMaterial::Clone()
	{
		NMaterial* clonedMaterial = new NMaterial();
		
		if(this->IsSwitchable())
		{
			HashMap<FastName, NMaterialState*>::Iterator stateIter = states.Begin();
			while(stateIter != states.End())
			{
				clonedMaterial->states.Insert(stateIter.GetKey(),
											  stateIter.GetValue()->CloneState());
				
				++stateIter;
			}
		}
		else
		{
			DeepCopyTo(clonedMaterial);
		}
		
		return clonedMaterial;
	}
    
    void NMaterial::SetMaterialName(const String& name)
    {
        if(IsSwitchable())
        {
            HashMap<FastName, NMaterialState*>::Iterator stateIter = states.Begin();
			while(stateIter != states.End())
			{
				stateIter.GetValue()->SetMaterialName(name);
				++stateIter;
			}
        }
        
        NMaterialState::SetMaterialName(name);
    }
	
	uint32 NMaterial::GetStateCount() const
	{
		return states.Size();
	}
	
	NMaterialState* NMaterial::GetState(uint32 index)
	{
		NMaterialState* matState = NULL;
		
		uint32 curIndex = 0;
		HashMap<FastName, NMaterialState*>::Iterator stateIter = states.Begin();
		while(stateIter != states.End() &&
			  curIndex < index)
		{
			++curIndex;
			++stateIter;
		}
		
		matState = stateIter.GetValue();

		return matState;
	}

};
