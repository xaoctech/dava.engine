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
#include "Render/Material/NMaterialTemplate.h"

namespace DAVA
{
    
	static const FastName DEFINE_VERTEX_LIT("VERTEX_LIT");
	static const FastName DEFINE_PIXEL_LIT("PIXEL_LIT");
    static const FastName LAYER_SHADOW_VOLUME("ShadowVolumeRenderLayer");
		
	/*MaterialTechnique::MaterialTechnique(const FastName & _shaderName, const FastNameSet & _uniqueDefines, RenderState * _renderState)
	{
		shader = 0;
		shaderName = _shaderName;
		uniqueDefines = _uniqueDefines;
		renderState = _renderState;
	}
    
	MaterialTechnique::~MaterialTechnique()
	{
		SafeRelease(shader);
        SafeDelete(renderState);
	}
	
	void MaterialTechnique::RecompileShader(const FastNameSet& materialDefines)
	{
		FastNameSet combinedDefines = materialDefines;
		
		if(uniqueDefines.size() > 0)
		{
			combinedDefines.Combine(uniqueDefines);
		}
		
		SafeRelease(shader);
		shader = SafeRetain(ShaderCache::Instance()->Get(shaderName, combinedDefines));
	}*/
    
	const FastName NMaterial::TEXTURE_ALBEDO("albedo");
	const FastName NMaterial::TEXTURE_NORMAL("normal");
	const FastName NMaterial::TEXTURE_DETAIL("detail");
	const FastName NMaterial::TEXTURE_LIGHTMAP("lightmap");
	const FastName NMaterial::TEXTURE_DECAL("decal");
	
	
	const FastName NMaterial::PARAM_LIGHT_POSITION0("lightPosition0");
	const FastName NMaterial::PARAM_PROP_AMBIENT_COLOR("prop_ambientColor");
	const FastName NMaterial::PARAM_PROP_DIFFUSE_COLOR("prop_diffuseColor");
	const FastName NMaterial::PARAM_PROP_SPECULAR_COLOR("prop_specularColor");
	const FastName NMaterial::PARAM_LIGHT_AMBIENT_COLOR("materialLightAmbientColor");
	const FastName NMaterial::PARAM_LIGHT_DIFFUSE_COLOR("materialLightDiffuseColor");
	const FastName NMaterial::PARAM_LIGHT_SPECULAR_COLOR("materialLightSpecularColor");
	const FastName NMaterial::PARAM_LIGHT_INTENSITY0("lightIntensity0");
	const FastName NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS("materialSpecularShininess");
	const FastName NMaterial::PARAM_FOG_COLOR("fogColor");
	const FastName NMaterial::PARAM_FOG_DENSITY("fogDensity");
	const FastName NMaterial::PARAM_FLAT_COLOR("flatColor");
	const FastName NMaterial::PARAM_TEXTURE0_SHIFT("texture0Shift");
	const FastName NMaterial::PARAM_UV_OFFSET("uvOffset");
	const FastName NMaterial::PARAM_UV_SCALE("uvScale");
		
	static FastName TEXTURE_NAME_PROPS[] = {
		NMaterial::TEXTURE_ALBEDO,
		NMaterial::TEXTURE_NORMAL,
		NMaterial::TEXTURE_DETAIL,
		NMaterial::TEXTURE_LIGHTMAP,
		NMaterial::TEXTURE_DECAL
	};
	
	
	void NMaterial::GenericPropertyManager::Init(NMaterialProperty* prop)
	{
		prop->type = Shader::UT_INT;
		prop->size = 0;
		prop->data = NULL;
	}
	
	void NMaterial::GenericPropertyManager::Release(NMaterialProperty* prop)
	{
		if(prop->data)
		{
			uint8* fakePtr = (uint8*)prop->data;
			SafeDeleteArray(fakePtr);
		}
	}
	
	NMaterialProperty* NMaterial::GenericPropertyManager::Clone(NMaterialProperty* prop)
	{
		NMaterial::GenericMaterialProperty* cloneProp = new NMaterial::GenericMaterialProperty();
		
		cloneProp->size = prop->size;
		cloneProp->type = prop->type;
		if(prop->data)
		{
			size_t dataSize = Shader::GetUniformTypeSize(prop->type) * prop->size;
			cloneProp->data = new uint8[dataSize];
			memcpy(cloneProp->data, prop->data, dataSize);
		}
		
		return cloneProp;
	}

	void NMaterial::UniformPropertyManager::Init(NMaterialProperty* prop)
	{
		prop->type = Shader::UT_INT;
		prop->size = 0;
		prop->data = NULL;
	}
	
	void NMaterial::UniformPropertyManager::Release(NMaterialProperty* prop)
	{
		//VI: do nothing here: memory is allocated in an external array
	}
	
	NMaterialProperty* NMaterial::UniformPropertyManager::Clone(NMaterialProperty* prop)
	{
		NMaterial::UniformMaterialProperty* cloneProp = new NMaterial::UniformMaterialProperty();
		cloneProp->size = prop->size;
		cloneProp->type = prop->type;
		cloneProp->data = prop->data;
		
		return cloneProp;
	}
	
////////////////////////////////////////////////////////////////////////////////
	
	NMaterial::NMaterial() :
	materialType(NMaterial::MATERIALTYPE_NONE),
	textureStateHandle(InvalidUniqueHandle),
	materialKey(0),
	texturesDirty(false),
	parent(NULL),
	requiredVertexFormat(EVF_FORCE_DWORD),
	lightCount(0),
	ready(false),
	illuminationParams(NULL),
	materialSetFlags(4),
	materialBlockedFlags(4),
	baseTechnique(NULL),
	activePassInstance(NULL),
	activeRenderPass(NULL)
	{
		memset(lights, 0, sizeof(lights));
	}
	
	NMaterial::~NMaterial()
	{
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			SafeDelete(it->second);
		}
		
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			SafeRelease(it->second->texture);
			SafeDelete(it->second);
		}

		ReleaseInstancePasses();
		
		if(InvalidUniqueHandle != textureStateHandle)
		{
			RenderManager::Instance()->ReleaseTextureStateData(textureStateHandle);
		}
		
		if(illuminationParams)
		{
			SafeDelete(illuminationParams);
		}
		
		SafeRelease(baseTechnique);
		
		if(parent)
		{
			parent->RemoveChild(this);
			
			SafeRelease(parent);
		}
	}
			
	void NMaterial::AddChild(NMaterial* material)
	{
		DVASSERT(std::find(children.begin(), children.end(), material) == children.end());
		
		children.push_back(material);
		material->OnParentChanged(this);
	}
	
	void NMaterial::RemoveChild(NMaterial* material)
	{
		Vector<NMaterial*>::iterator child = std::find(children.begin(), children.end(), material);
		if(children.end() != child)
		{
			children.erase(child);
			material->OnParentChanged(NULL);
		}
	}
	
    int32 NMaterial::GetChildrenCount() const
	{
		return children.size();
	}
	
    NMaterial* NMaterial::GetChild(int32 index) const
	{
		DVASSERT(index >= 0 && index < children.size());
		return children[index];
	}
		
	void NMaterial::SetFlag(const FastName& flag, bool flagValue)
	{
		if(false == IsFlagBlocked(flag))
		{
			if(flagValue)
			{
				materialSetFlags.Insert(flag);
			}
			else
			{
				materialSetFlags.erase(flag);
			}
			
			size_t childrenCount = children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				children[i]->OnParentFlagsChanged();
			}
		}
	}
	
	void NMaterial::SetBlockFlag(const FastName& flag, bool blockState)
	{
		if(blockState)
		{
			materialBlockedFlags.Insert(flag);
		}
		else
		{
			materialBlockedFlags.erase(flag);
		}
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->OnParentFlagsChanged();
		}
	}
		
	bool NMaterial::GetFlagValue(const FastName& flag)
	{
		bool result = (materialSetFlags.at(flag) != 0);
		
		if(!result &&
		   parent != NULL)
		{
			result = parent->GetFlagValue(flag);
		}
		
		return result;

	}
	
	bool NMaterial::IsFlagBlocked(const FastName& flag)
	{
		bool result = (materialBlockedFlags.at(flag) != 0);
		
		if(!result &&
		   parent != NULL)
		{
			result = parent->IsFlagBlocked(flag);
		}
		
		return result;
	}
		
	void NMaterial::Save(KeyedArchive * archive,
						 SerializationContext* serializationContext)
	{
		archive->SetString("materialName", (materialName.IsValid()) ? materialName.c_str() : "");
		archive->SetInt32("materialType", (int32)materialType);
		int64 tmpMaterialKey = (int64)materialKey;
		archive->SetInt64("materialKey", tmpMaterialKey);
		
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType &&
		   parent)
		{
			tmpMaterialKey = (int64)parent->materialKey;
			archive->SetInt64("parentMaterialKey", tmpMaterialKey);
		}
		
		KeyedArchive* materialTextures = new KeyedArchive();
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			if(it->second->texture)
			{
				materialTextures->SetString(it->first.c_str(),
											it->second->path.GetRelativePathname(serializationContext->GetScenePath()));
			}
		}
		archive->SetArchive("textures", materialTextures);
		SafeRelease(materialTextures);
		
		KeyedArchive* materialProps = new KeyedArchive();
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			NMaterialProperty* property = it->second;
			
			uint32 dataSize = Shader::GetUniformTypeSize(property->type) * property->size;
			uint8* propertyStorage = new uint8[dataSize + sizeof(property->type) + sizeof(property->size)];
			
			memcpy(propertyStorage, &property->type, sizeof(property->type));
			memcpy(propertyStorage + sizeof(property->type), &property->size, sizeof(property->size));
			memcpy(propertyStorage + sizeof(property->type) + sizeof(property->size), property->data, dataSize);
			
			materialProps->SetByteArray(it->first.c_str(), propertyStorage, dataSize + sizeof(property->type) + sizeof(property->size));
			
			SafeDeleteArray(propertyStorage);
		}
		archive->SetArchive("properties", materialProps);
		SafeRelease(materialProps);
		
		KeyedArchive* materialSetFlagsArchive = new KeyedArchive();
		SerializeFastNameSet(materialSetFlags, materialSetFlagsArchive);
		archive->SetArchive("setFlags", materialSetFlagsArchive);
		SafeRelease(materialSetFlagsArchive);
		
		KeyedArchive* materialBlockedFlagsArchive = new KeyedArchive();
		SerializeFastNameSet(materialBlockedFlags, materialBlockedFlagsArchive);
		archive->SetArchive("blockedFlags", materialBlockedFlagsArchive);
		SafeRelease(materialBlockedFlagsArchive);
		
		if(illuminationParams)
		{
			archive->SetBool("illumination.isUsed", illuminationParams->isUsed);
			archive->SetBool("illumination.castShadow", illuminationParams->castShadow);
			archive->SetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
			archive->SetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
		}
	}
	
	void NMaterial::Load(KeyedArchive * archive,
						 SerializationContext* serializationContext)
	{
		materialName = FastName(archive->GetString("materialName"));
		materialType = (NMaterial::eMaterialType)archive->GetInt32("materialType");
		materialKey = (NMaterial::NMaterialKey)archive->GetInt64("materialKey");
		
		DataNode::SetName(materialName.c_str());
		
		const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = propsMap.begin();
			it != propsMap.end();
			++it)
		{
			const VariantType* propVariant = it->second;
			DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
			DVASSERT(propVariant->AsByteArraySize() >= (sizeof(uint32) +sizeof(uint32)));
			
			const uint8* ptr = propVariant->AsByteArray();
			
			SetPropertyValue(FastName(it->first),
							 *(Shader::eUniformType*)ptr,
							 *(ptr + sizeof(Shader::eUniformType)),
							 ptr + sizeof(Shader::eUniformType) + sizeof(uint8));
		}
		
		const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = texturesMap.begin();
			it != texturesMap.end();
			++it)
		{
			String relativePathname = it->second->AsString();
			SetTexture(FastName(it->first), relativePathname);
		}
		
		DeserializeFastNameSet(archive->GetArchive("layers"), materialSetFlags);
		DeserializeFastNameSet(archive->GetArchive("nativeDefines"), materialBlockedFlags);
		
		if(archive->IsKeyExists("illumination.isUsed"))
		{
			illuminationParams = new IlluminationParams();
			
			illuminationParams->isUsed = archive->GetBool("illumination.isUsed", illuminationParams->isUsed);
			illuminationParams->castShadow = archive->GetBool("illumination.castShadow", illuminationParams->castShadow);
			illuminationParams->receiveShadow = archive->GetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
			illuminationParams->lightmapSize = archive->GetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
		}
		
		serializationContext->SetMaterial((uint64)materialKey, this);
		
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			NMaterial::NMaterialKey parentKey = (NMaterial::NMaterialKey)archive->GetInt64("parentMaterialKey");
			NMaterial* parent = serializationContext->GetMaterial((uint64)parentKey);
			
			if(parent)
			{
				parent->AddChild(this);
			}
		}
	}
	
	void NMaterial::DeserializeFastNameSet(const KeyedArchive* srcArchive,
										   FastNameSet& targetSet)
	{
		const Map<String, VariantType*>& setData = srcArchive->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = setData.begin();
			it != setData.end();
			++it)
		{
			targetSet.Insert(FastName(it->first));
		}
	}
	
	void NMaterial::SerializeFastNameSet(const FastNameSet& srcSet,
										 KeyedArchive* targetArchive)
	{
		for(FastNameSet::iterator it = srcSet.begin();
			it != srcSet.end();
			++it)
		{
			targetArchive->SetBool(it->first.c_str(), true);
		}
	}
	
	bool NMaterial::SwitchQuality(const FastName& stateName)
	{
		bool result = false;
		
		currentQuality = stateName;
		
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			OnInstanceQualityChanged();
		}
		else if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
		{
			DVASSERT(materialTemplate);
			
			size_t childrenCount = children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				children[i]->SwitchQuality(stateName);
			}
		}
		else
		{
			DVASSERT(false && "Material is not initialized propely!");
		}
		
		return result;
	}
		
	NMaterial* NMaterial::Clone()
	{
		NMaterial* clonedMaterial = NULL;
		if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
		{
			clonedMaterial = MaterialSystem::CreateMaterial(materialName,
															materialTemplate->name);
			
		}
		else if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			clonedMaterial = MaterialSystem::CreateMaterialInstance();

			SafeRetain(parent);
		}
		else
		{
			DVASSERT(false && "Material is not initialized properly!");
			return clonedMaterial;
		}
				
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			clonedMaterial->materialProperties.insert(it->first, it->second->Clone());
		}
		
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			clonedMaterial->SetTexture(it->first, it->second->path);
		}
		
		if(illuminationParams)
		{
			clonedMaterial->illuminationParams = new IlluminationParams();
			clonedMaterial->illuminationParams->castShadow = illuminationParams->castShadow;
			clonedMaterial->illuminationParams->isUsed = illuminationParams->isUsed;
			clonedMaterial->illuminationParams->lightmapSize = illuminationParams->lightmapSize;
			clonedMaterial->illuminationParams->receiveShadow = illuminationParams->receiveShadow;
		}
		
		clonedMaterial->materialSetFlags.Combine(materialSetFlags);
		clonedMaterial->materialBlockedFlags.Combine(materialBlockedFlags);

		return clonedMaterial;
	}
	
	NMaterial* NMaterial::Clone(const String& newName)
	{
		NMaterial* clonedMaterial = Clone();
		clonedMaterial->SetName(newName);
		clonedMaterial->SetMaterialName(newName);
		clonedMaterial->SetMaterialKey((NMaterial::NMaterialKey)clonedMaterial);
		
		return clonedMaterial;
	}
    
	IlluminationParams * NMaterial::GetIlluminationParams()
    {
        if(!illuminationParams)
        {
            illuminationParams = new IlluminationParams();
            illuminationParams->SetDefaultParams();
        }
        return illuminationParams;
    }
	
    void NMaterial::ReleaseIlluminationParams()
    {
        SafeDelete(illuminationParams);
    }
	
    void NMaterial::SetTexture(const FastName& textureFastName,
							   const FilePath& texturePath)
	{
		TextureBucket* bucket = textures.at(textureFastName);
		if(NULL == bucket)
		{
			bucket = new TextureBucket();
			textures.insert(textureFastName, bucket);
		}
		
		if(bucket->path != texturePath)
		{
			SafeRelease(bucket->texture);
			bucket->path = texturePath;
			
			texturesDirty = true;
		}
	}
	
	void NMaterial::SetTexture(const FastName& textureFastName,
							   Texture* texture)
	{
		TextureBucket* bucket = textures.at(textureFastName);
		if(NULL == bucket)
		{
			bucket = new TextureBucket();
			textures.insert(textureFastName, bucket);
		}

		if(texture != bucket->texture)
		{
			SafeRelease(bucket->texture);
			
			bucket->texture = SafeRetain(texture);
			bucket->path = texture->relativePathname;
			
			texturesDirty = true;
		}
	}
	
    Texture * NMaterial::GetTexture(const FastName& textureFastName) const
	{
		TextureBucket* bucket = textures.at(textureFastName);
		return (NULL == bucket) ? NULL : bucket->texture;
	}
	
	const FilePath& NMaterial::GetTexturePath(const FastName& textureFastName) const
	{
		static FilePath invalidEmptyPath;
		TextureBucket* bucket = textures.at(textureFastName);
		return (NULL == bucket) ? invalidEmptyPath : bucket->path;
	}
	
    Texture * NMaterial::GetTexture(int32 index) const
	{
		DVASSERT(index >= 0 && index < textures.size());
		
		TextureBucket* bucket = textures.valueByIndex(index);
		return bucket->texture;
	}
	
	const FilePath& NMaterial::GetTexturePath(int32 index) const
	{
		DVASSERT(index >= 0 && index < textures.size());
		
		TextureBucket* bucket = textures.valueByIndex(index);
		return bucket->path;
	}
	
	const FastName& NMaterial::GetTextureName(int32 index) const
	{
		DVASSERT(index >= 0 && index < textures.size());
		
		return textures.keyByIndex(index);
	}
	
    uint32 NMaterial::NMaterial::GetTextureCount() const
	{
		return textures.size();
	}
    
    void NMaterial::SetPropertyValue(const FastName & keyName,
						  Shader::eUniformType type,
						  uint32 size,
						  const void * data)
	{
		size_t dataSize = Shader::GetUniformTypeSize(type) * size;
		NMaterialProperty * materialProperty = materialProperties.at(keyName);
		if (materialProperty)
		{
			if (materialProperty->type != type || materialProperty->size != size)
			{
				//VI: material property type or size chnage should never happen at runtime
				DVASSERT(false && "Runtime change of material property type!");
				
				NMaterialProperty* newProp = new NMaterial::GenericMaterialProperty();
				newProp->size = size;
				newProp->type = type;
				newProp->data = new uint8[dataSize];
				
				SafeDelete(materialProperty);
				materialProperties.insert(keyName, newProp);
				
				materialProperty = newProp;
			}
		}
		else
		{
			materialProperty = new NMaterial::GenericMaterialProperty();
			materialProperty->size = size;
			materialProperty->type = type;
			materialProperty->data = new uint8[dataSize];
			materialProperties.insert(keyName, materialProperty);
		}
		
		memcpy(materialProperty->data, data, dataSize);
	}
	
	NMaterialProperty* NMaterial::GetPropertyValue(const FastName & keyName) const
	{
		const NMaterial* currentMaterial = this;
		NMaterialProperty * property = NULL;
		while(currentMaterial != NULL)
		{
			property = currentMaterial->materialProperties.at(keyName);
			if (property)
			{
				break;
			}
			
			currentMaterial = currentMaterial->parent;
		}
		
		return property;
	}
	
	NMaterialProperty* NMaterial::GetMaterialProperty(const FastName & keyName) const
	{
		return materialProperties.at(keyName);
	}
	
	void NMaterial::RemoveMaterialProperty(const FastName & keyName)
	{
		materialProperties.erase(keyName);
	}
	
	void NMaterial::SetMaterialName(const String& name)
	{
		materialName = FastName(name);
	}
	
	void NMaterial::SetMaterialTemplate(const NMaterialTemplate* matTemplate)
	{
		materialTemplate = matTemplate;
		
		OnMaterialTemplateChanged();
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->SetMaterialTemplate(matTemplate);
		}
	}
	
	const FastNameSet& GetRenderLayers()
	{
		DVASSERT(false && "Implement render layers!")
	}
	
	void NMaterial::OnMaterialTemplateChanged()
	{
		UpdateMaterialTemplate();
	}

	void NMaterial::OnParentChanged(NMaterial* newParent)
	{
		NMaterial* oldParent = parent;
		parent = SafeRetain(newParent);
		SafeRelease(oldParent);
		
		SetMaterialTemplate(newParent->materialTemplate);
	}
	
	void NMaterial::OnParentFlagsChanged()
	{
		UpdateMaterialTemplate();
	}
	
	void NMaterial::OnInstanceQualityChanged()
	{
		UpdateMaterialTemplate();
	}
	
	void NMaterial::BuildEffectiveFlagSet(FastNameSet effectiveFlagSet)
	{
		effectiveFlagSet.clear();
		FastNameSet effectiveBlockedFlags(8);
		
		NMaterial* currentMaterial = this;
		while(NULL != currentMaterial)
		{
			effectiveFlagSet.Combine(currentMaterial->materialSetFlags);
			effectiveBlockedFlags.Combine(currentMaterial->materialBlockedFlags);
			
			currentMaterial = currentMaterial->parent;
		}
		
		if(effectiveBlockedFlags.size() > 0)
		{
			for(FastNameSet::iterator it = effectiveBlockedFlags.begin();
				it != effectiveBlockedFlags.end();
				++it)
			{
				effectiveFlagSet.erase(it->first);
			}
		}
	}
	
	void NMaterial::ReleaseInstancePasses()
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* passInstance = it->second;
			
			SafeRelease(passInstance->shader);
			
			if(passInstance->dirtyState &&
			   passInstance->renderState != InvalidUniqueHandle)
			{
				RenderManager::Instance()->ReleaseRenderStateData(passInstance->renderState);
			}
			
			if(passInstance->textureParamsCachePtr)
			{
				for(size_t i = 0; i < passInstance->textureParamsCacheSize; ++i)
				{
					SafeRelease(passInstance->textureParamsCachePtr[i].tx);
				}
			}
			
			SafeDelete(it->second);
		}
	}
	
	void NMaterial::UpdateMaterialTemplate()
	{
		DVASSERT(materialTemplate);
		
		ReleaseInstancePasses();
		
		FastNameSet effectiveFlags;
		BuildEffectiveFlagSet(effectiveFlags);
		
		FastName techniqueName;
		if(currentQuality.IsValid())
		{
			techniqueName = materialTemplate->techniqueStateMap.at(currentQuality);
		}
		
		if(!techniqueName.IsValid())
		{
			techniqueName = materialTemplate->techniqueStateMap.valueByIndex(0);
		}
		
		DVASSERT(techniqueName.IsValid());
		
		SafeRelease(baseTechnique);
		baseTechnique = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(techniqueName.c_str());
		
		DVASSERT(baseTechnique);
		
		uint32 passCount = baseTechnique->GetPassCount();
		for(uint32 i = 0; i < passCount; ++i)
		{
			RenderTechniquePass* pass = baseTechnique->GetPassByIndex(i);
			UpdateRenderPass(baseTechnique->GetPassName(i), effectiveFlags, pass);
		}
	}
	
	void NMaterial::UpdateRenderPass(const FastName& passName,
									 const FastNameSet& instanceDefines,
									 RenderTechniquePass* pass)
	{
		RenderPassInstance* passInstance = new RenderPassInstance();
		passInstance->dirtyState = false;
		passInstance->shader = pass->RetainShader(instanceDefines);
		passInstance->renderState = pass->GetRenderState()->stateHandle;
		
		instancePasses.insert(passName, passInstance);
		
		//VI: BuildTextureParamsCache will add textures that are required for the shader
		//VI: but has not been set up yet. Also it will add reference for all active textures
		BuildTextureParamsCache(passInstance);
	}
	
	void NMaterial::BuildTextureParamsCache(RenderPassInstance* passInstance)
	{
		Shader * shader = passInstance->shader;
		
		uint32 uniformCount = shader->GetUniformCount();
		for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			if(uniform->id == Shader::UNIFORM_NONE)
			{
				if(uniform->type == Shader::UT_SAMPLER_2D ||
				   uniform->type == Shader::UT_SAMPLER_CUBE)
				{
					TextureParamCacheEntry entry;
					entry.textureName = uniform->name;
					DVASSERT(false && "Implement slot taking from shader!");
					entry.slot = 0;
					entry.tx = NULL;
										
					passInstance->textureParamsCache.push_back(entry);
				}
			}
		}
		
		passInstance->textureParamsCachePtr = NULL;
		passInstance->textureParamsCacheSize = 0;
		if(passInstance->textureParamsCache.size())
		{
			passInstance->textureParamsCachePtr = &passInstance->textureParamsCache[0];
			passInstance->textureParamsCacheSize = passInstance->textureParamsCache.size();
		}
	}
	
	void NMaterial::BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance)
	{
		
	}

	NMaterial::TextureBucket* NMaterial::GetTextureBucketRecursive(const FastName& textureFastName) const
	{
		TextureBucket* bucket = NULL;
		const NMaterial* currentMaterial = this;
		while(currentMaterial)
		{
			bucket = currentMaterial->textures.at(textureFastName);
			
			currentMaterial = currentMaterial->parent;
		}
		
		return bucket;
	}
};
