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


#include "Scene3D/Systems/MaterialSystem.h"
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
	
	const FastName NMaterial::FLAG_VERTEXFOG = FastName("VERTEX_FOG");
	const FastName NMaterial::FLAG_TEXTURESHIFT = FastName("TEXTURE0_SHIFT_ENABLED");
	const FastName NMaterial::FLAG_FLATCOLOR = FastName("FLATCOLOR");
	const FastName NMaterial::FLAG_LIGHTMAPONLY = FastName("MATERIAL_VIEW_LIGHTMAP_ONLY");
	const FastName NMaterial::FLAG_TEXTUREONLY = FastName("MATERIAL_VIEW_TEXTURE_ONLY");
	const FastName NMaterial::FLAG_SETUPLIGHTMAP = FastName("SETUP_LIGHTMAP");

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
	
////////////////////////////////////////////////////////////////////////////////
	
	NMaterial::NMaterial() :
	materialType(NMaterial::MATERIALTYPE_NONE),
	materialKey(0),
	parent(NULL),
	requiredVertexFormat(EVF_FORCE_DWORD),
	lightCount(0),
	illuminationParams(NULL),
	materialSetFlags(8),
	baseTechnique(NULL),
	activePassInstance(NULL),
	activeRenderPass(NULL),
	instancePasses(4),
	textures(8)
	{
		memset(lights, 0, sizeof(lights));
	}
	
	NMaterial::~NMaterial()
	{
		ReleaseInstancePasses();
		
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			SafeDelete(it->second);
		}
		materialProperties.clear();
		
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			SafeRelease(it->second->texture);
			SafeDelete(it->second);
		}
		textures.clear();
				
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
		
		this->Retain();
		
		material->OnParentChanged(this);
		
		this->Release();
	}
	
	void NMaterial::RemoveChild(NMaterial* material)
	{
		Vector<NMaterial*>::iterator child = std::find(children.begin(), children.end(), material);
		if(children.end() != child)
		{
			this->Retain();
			
			children.erase(child);
			material->OnParentChanged(NULL);
			
			//VI: TODO: review if this call is realy needed at this point
			CleanupUnusedTextures();
		
			this->Release();
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
		
	void NMaterial::SetFlag(const FastName& flag, eFlagValue flagValue)
	{
		materialSetFlags.insert(flag, flagValue);

		UpdateShaderWithFlags();
		
		this->Retain();
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->OnParentFlagsChanged();
		}
				
		this->Release();
	}
			
	int32 NMaterial::GetFlagValue(const FastName& flag) const
	{
		int32 result = materialSetFlags.at(flag);
		
		if(NMaterial::FlagOff == result && parent)
		{
			result = parent->GetFlagValue(flag);
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
		for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
			it != materialSetFlags.end();
			++it)
		{
			materialSetFlagsArchive->SetInt32(it->first.c_str(), it->second);
		}
		archive->SetArchive("setFlags", materialSetFlagsArchive);
		SafeRelease(materialSetFlagsArchive);
				
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
		
		const Map<String, VariantType*>& flagsMap = archive->GetArchive("setFlags")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = flagsMap.begin();
			it != flagsMap.end();
			++it)
		{
			SetFlag(FastName(it->first), (NMaterial::eFlagValue)it->second->AsInt32());
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
			
			UpdateMaterialTemplate();
			
			LoadActiveTextures();
			
			this->Retain();
			
			size_t childrenCount = children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				children[i]->SwitchQuality(stateName);
			}
			
			//VI: TODO: review if this call is realy needed at this point
			CleanupUnusedTextures();
			
			this->Release();
		}
		else
		{
			DVASSERT(false && "Material is not initialized properly!");
		}
		
		return result;
	}
		
	NMaterial* NMaterial::Clone()
	{
		NMaterial* clonedMaterial = NULL;
		if(NMaterial::MATERIALTYPE_MATERIAL == materialType)
		{
			clonedMaterial = MaterialSystem::CreateMaterial(materialName,
															materialTemplate->name,
															currentQuality);
			
		}
		else if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			clonedMaterial = MaterialSystem::CreateMaterialInstance();
		}
		else
		{
			DVASSERT(false && "Material is not initialized properly!");
			return clonedMaterial;
		}
		
		clonedMaterial->SetName(GetName());
				
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
				
		if(NMaterial::MATERIALTYPE_INSTANCE == materialType)
		{
			parent->AddChild(clonedMaterial);
		}
		
		for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
			it != materialSetFlags.end();
			++it)
		{
			clonedMaterial->SetFlag(it->first, (NMaterial::eFlagValue)it->second);
		}
		
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* currentPass = it->second;
			
			if(currentPass->dirtyState)
			{
				RenderPassInstance* clonedPass = clonedMaterial->instancePasses.at(it->first);
				DVASSERT(clonedPass);
				
				clonedPass->dirtyState = true;
				clonedPass->renderState.stateHandle = currentPass->renderState.stateHandle;
				RenderManager::Instance()->RetainRenderStateData(currentPass->renderState.stateHandle);
			}
		}

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
			
			if(IsTextureActive(textureFastName))
			{
				bucket->texture = Texture::CreateFromFile(texturePath);
			}
			
			SetTexturesDirty();
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
			
			SetTexturesDirty();
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
				
				OnMaterialPropertyRemoved(keyName);
				
				SafeDelete(materialProperty);
				materialProperties.insert(keyName, newProp);
				
				OnMaterialPropertyAdded(keyName, newProp);
				
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
			
			OnMaterialPropertyAdded(keyName, materialProperty);
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
		OnMaterialPropertyRemoved(keyName);
		
		NMaterialProperty* prop = materialProperties.at(keyName);
		if(prop)
		{
			materialProperties.erase(keyName);
			SafeDelete(prop);
		}
	}
	
	void NMaterial::SetMaterialName(const String& name)
	{
		materialName = FastName(name);
	}
	
	void NMaterial::SetMaterialTemplate(const NMaterialTemplate* matTemplate,
										const FastName& defaultQuality)
	{
		materialTemplate = matTemplate;
		currentQuality = defaultQuality;
		
		OnMaterialTemplateChanged();
		
		LoadActiveTextures();
		
		this->Retain();
				
		//{VI: temporray code should be removed once lighting system is up
		materialDynamicLit = (baseTechnique->GetLayersSet().count(LAYER_SHADOW_VOLUME) != 0);
		
		if(!materialDynamicLit)
		{
			uint32 passCount = baseTechnique->GetPassCount();
			for(uint32 i = 0; i < passCount; ++i)
			{
				RenderTechniquePass* pass = baseTechnique->GetPassByIndex(i);
				const FastNameSet& defines = pass->GetUniqueDefineSet();
				materialDynamicLit = materialDynamicLit ||
				defines.count(DEFINE_VERTEX_LIT) ||
				defines.count(DEFINE_PIXEL_LIT);
			}
		}
		//}
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->SetMaterialTemplate(matTemplate, defaultQuality);
		}
		
		//VI: TODO: review if this call is realy needed at this point
		CleanupUnusedTextures();
		
		this->Release();
	}
	
	const FastNameSet& NMaterial::GetRenderLayers()
	{
		DVASSERT(baseTechnique);
		
		return baseTechnique->GetLayersSet();
	}
	
	void NMaterial::OnMaterialTemplateChanged()
	{
		UpdateMaterialTemplate();
	}

	void NMaterial::OnParentChanged(NMaterial* newParent)
	{
		bool oldParentHadTextures = false;
		bool newParentHasTextures = false;
		if(parent)
		{
			oldParentHadTextures = (parent->GetTextureCount() > 0);
		}
		
		NMaterial* oldParent = parent;
		parent = SafeRetain(newParent);
		SafeRelease(oldParent);
		
		if(newParent)
		{
			newParentHasTextures = (newParent->GetTextureCount() > 0);
			SetMaterialTemplate(newParent->materialTemplate, newParent->currentQuality);
		}
		
		SetTexturesDirty();
	}
	
	void NMaterial::OnParentFlagsChanged()
	{
		UpdateShaderWithFlags();
	}
	
	void NMaterial::OnInstanceQualityChanged()
	{
		UpdateMaterialTemplate();
		
		LoadActiveTextures();
		
		//VI: TODO: review if this call is realy needed at this point
		CleanupUnusedTextures();
	}
	
	void NMaterial::BuildEffectiveFlagSet(FastNameSet& effectiveFlagSet)
	{
		effectiveFlagSet.clear();
				
		HashMap<FastName, int32> rawEffectiveFlags;
		BuildEffectiveFlagSet(rawEffectiveFlags);
		
		for(HashMap<FastName, int32>::iterator it = rawEffectiveFlags.begin();
			it != rawEffectiveFlags.end();
			++it)
		{
			int32 currentFlagValue = it->second;
			if(NMaterial::FlagOn == currentFlagValue ||
			   NMaterial::FlagOnOverride == currentFlagValue)
			{
				effectiveFlagSet.Insert(it->first);
			}
		}
	}
					
	void NMaterial::BuildEffectiveFlagSet(HashMap<FastName, int32>& effectiveFlagSet)
	{
		if(parent)
		{
			parent->BuildEffectiveFlagSet(effectiveFlagSet);
		}

		for(HashMap<FastName, int32>::iterator it = materialSetFlags.begin();
			it != materialSetFlags.end();
			++it)
		{
			int32 currentFlagValue = it->second;
			int32 effectiveFlagValue = effectiveFlagSet.at(it->first);
			
			if(NMaterial::FlagOnOverride == currentFlagValue ||
			   NMaterial::FlagOffOverride == currentFlagValue ||
			   (NMaterial::FlagOn == currentFlagValue &&
				NMaterial::FlagOffOverride != effectiveFlagValue))
			{
				effectiveFlagSet.insert(it->first, currentFlagValue);
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
			
			//VI: TODO: make sure need to release shader
			//SafeRelease(passInstance->renderState.shader);
			
			if(passInstance->dirtyState &&
			   passInstance->renderState.stateHandle != InvalidUniqueHandle)
			{
				RenderManager::Instance()->ReleaseRenderStateData(passInstance->renderState.stateHandle);
			}
						
			if(passInstance->renderState.textureState != InvalidUniqueHandle)
			{
				RenderManager::Instance()->ReleaseTextureStateData(passInstance->renderState.textureState);
			}
			
			SafeDelete(it->second);
		}
		instancePasses.clear();
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
		baseTechnique = RenderTechniqueSingleton::Instance()->RetainRenderTechniqueByName(techniqueName);
		
		DVASSERT(baseTechnique);
		
		//TODO: add unused render passes removing
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
		RenderState* parentRenderState = pass->GetRenderState();
		
		RenderPassInstance* passInstance = new RenderPassInstance();
		passInstance->dirtyState = false;
		passInstance->renderState.stateHandle = parentRenderState->stateHandle;
		passInstance->renderState.textureState = InvalidUniqueHandle;
		passInstance->texturesDirty = false;
		passInstance->renderState.shader = pass->RetainShader(instanceDefines);
		passInstance->renderState.scissorRect = parentRenderState->scissorRect;
		passInstance->renderState.renderer = parentRenderState->renderer;
		passInstance->renderState.color = parentRenderState->color;
		
		instancePasses.insert(passName, passInstance);
		
		BuildTextureParamsCache(passInstance);
		BuildActiveUniformsCacheParamsCache(passInstance);
	}
	
	void NMaterial::BuildTextureParamsCache(RenderPassInstance* passInstance)
	{
		Shader* shader = passInstance->renderState.shader;
		
		uint32 uniformCount = shader->GetUniformCount();
		for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			if(uniform->id == Shader::UNIFORM_NONE)
			{
				if(uniform->type == Shader::UT_SAMPLER_2D ||
				   uniform->type == Shader::UT_SAMPLER_CUBE)
				{
					ShaderAsset* asset = ShaderCache::Instance()->Get(shader->GetAssetName());
					DVASSERT(asset);
					int32 textureIndex = asset->GetDefaultValue(uniform->name).int32Value;
					passInstance->textureIndexMap.insert(uniform->name, textureIndex);
				}
			}
		}
	}
	
	void NMaterial::BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance)
	{
		Shader* shader = passInstance->renderState.shader;
		passInstance->activeUniformsCache.clear();
		
		uint32 uniformCount = shader->GetUniformCount();
		for(uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			
			if(Shader::UNIFORM_NONE == uniform->id ||
			   Shader::UNIFORM_COLOR == uniform->id) //TODO: do something with conditional binding
			{
				NMaterialProperty* prop = GetPropertyValue(uniform->name);
				
				UniformCacheEntry entry;
				entry.uniform = uniform;
				entry.index = uniformIndex;
				entry.prop = prop;
				
				passInstance->activeUniformsCache.push_back(entry);
			}
		}
		
		passInstance->activeUniformsCachePtr = NULL;
		passInstance->activeUniformsCacheSize = 0;
		if(passInstance->activeUniformsCache.size())
		{
			passInstance->activeUniformsCachePtr = &passInstance->activeUniformsCache[0];
			passInstance->activeUniformsCacheSize = passInstance->activeUniformsCache.size();
		}
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
	
	void NMaterial::LoadActiveTextures()
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			
			for(HashMap<FastName, int32>::iterator texIt = pass->textureIndexMap.begin();
				texIt != pass->textureIndexMap.end();
				++texIt)
			{
				GetOrLoadTextureRecursive(texIt->first);
			}
		}
	}
	
	void NMaterial::CleanupUnusedTextures()
	{
		for(HashMap<FastName, TextureBucket*>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			if(!IsTextureActive(it->first))
			{
				SafeRelease(it->second->texture);
			}
		}
		
		if(parent)
		{
			parent->CleanupUnusedTextures();
		}
	}
	
	Texture* NMaterial::GetOrLoadTextureRecursive(const FastName& textureName)
	{
		Texture* tex = NULL;
		
		NMaterial* currentMaterial = this;
		while(currentMaterial)
		{
			TextureBucket* bucket = currentMaterial->textures.at(textureName);
			
			if(bucket)
			{
				if(NULL == bucket->texture)
				{
					bucket->texture = Texture::CreateFromFile(bucket->path);
				}
				
				tex = bucket->texture;
				break;
			}
			
			currentMaterial = currentMaterial->parent;
		}
		
		return tex;
	}
	
	bool NMaterial::IsTextureActive(const FastName& textureName) const
	{
		bool active = false;
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			if(pass->textureIndexMap.count(textureName) > 0)
			{
				active = true;
				break;
			}
		}
		
		if(!active)
		{
			size_t childrenCount = children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				active = children[i]->IsTextureActive(textureName);
				if(active)
				{
					break;
				}
			}
		}
		
		return active;
	}
	
	void NMaterial::SetTexturesDirty()
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			pass->texturesDirty = true;
		}
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i < childrenCount; ++i)
		{
			children[i]->SetTexturesDirty();
		}
	}
	
	void NMaterial::PrepareTextureState(RenderPassInstance* passInstance)
	{
		DVASSERT(passInstance);
		
		TextureStateData textureData;
		for(HashMap<FastName, int32>::iterator texIt = passInstance->textureIndexMap.begin();
			texIt != passInstance->textureIndexMap.end();
			++texIt)
		{
			textureData.textures[texIt->second] = GetOrLoadTextureRecursive(texIt->first);
			if(NULL == textureData.textures[texIt->second])
			{
				//VI: this case is mostly for ResEditor
				textureData.textures[texIt->second] = Texture::CreatePink();
			}
		}
		
		//VI: this strange code ensures we do not release textures prematurely
		//VI: in case if there's such texture state already and it's owned by us only
		UniqueHandle oldHandle = passInstance->renderState.textureState;
		passInstance->renderState.textureState = RenderManager::Instance()->AddTextureStateData(&textureData);
		
		if(InvalidUniqueHandle != oldHandle)
		{
			RenderManager::Instance()->ReleaseTextureStateData(oldHandle);
		}
		
		passInstance->texturesDirty = false;
		
		for(int i = 0; i < COUNT_OF(textureData.textures); ++i)
		{
			if(textureData.textures[i] &&
			   textureData.textures[i]->isPink)
			{
				SafeRelease(textureData.textures[i]);
			}
		}
	}
	
	void NMaterial::BindMaterialTechnique(const FastName & passName, Camera* camera)
	{
		if(activePassName != passName)
		{
			activePassName = passName;
			activeRenderPass = baseTechnique->GetPassByName(passName);
			activePassInstance = instancePasses.at(passName);
			
			DVASSERT(activeRenderPass);
			DVASSERT(activePassInstance);
		}
		
		//VI: this call is temporary solution. It will be removed once autobind system and lighting system ready
		SetupPerFrameProperties(camera);
		
		BindMaterialTextures(activePassInstance);
		
		RenderManager::Instance()->FlushState(&activePassInstance->renderState);
		
		BindMaterialProperties(activePassInstance);
	}
	
	void NMaterial::SetupPerFrameProperties(Camera* camera)
	{
		if(camera && IsDynamicLit() && lights[0])
		{
			const Matrix4 & matrix = camera->GetMatrix();
			Vector3 lightPosition0InCameraSpace = lights[0]->GetPosition() * matrix;
			
			SetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0, Shader::UT_FLOAT_VEC3, 1, lightPosition0InCameraSpace.data);
		}
	}

	void NMaterial::BindMaterialTextures(RenderPassInstance* passInstance)
	{
		if(activePassInstance->texturesDirty)
		{
			PrepareTextureState(activePassInstance);
		}
	}
	
	void NMaterial::BindMaterialProperties(RenderPassInstance* passInstance)
	{
		Shader* shader = passInstance->renderState.shader;
		for(size_t i = 0; i < passInstance->activeUniformsCacheSize; ++i)
		{
			UniformCacheEntry& uniformEntry = passInstance->activeUniformsCachePtr[i];
			Shader::Uniform* uniform = uniformEntry.uniform;
			
			if(uniformEntry.prop)
			{
				shader->SetUniformValueByUniform(uniform,
												 uniform->type,
												 uniform->size,
												 uniformEntry.prop->data);
			}
		}
	}
	
	void NMaterial::OnMaterialPropertyAdded(const FastName& propName,
											NMaterialProperty* prop)
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			if(pass->activeUniformsCachePtr)
			{
				for(size_t i = 0; i < pass->activeUniformsCacheSize; ++i)
				{
					if(pass->activeUniformsCachePtr[i].uniform->name == propName)
					{
						pass->activeUniformsCachePtr[i].prop = prop;
						break;
					}
				}
			}
		}
		
		this->Retain();
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i< childrenCount; ++i)
		{
			children[i]->OnMaterialPropertyAdded(propName, prop);
		}
		
		this->Release();
	}
	
	void NMaterial::OnMaterialPropertyRemoved(const FastName& propName)
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			RenderPassInstance* pass = it->second;
			if(pass->activeUniformsCachePtr)
			{
				for(size_t i = 0; i < pass->activeUniformsCacheSize; ++i)
				{
					if(pass->activeUniformsCachePtr[i].uniform->name == propName)
					{
						pass->activeUniformsCachePtr[i].prop = NULL;
						break;
					}
				}
			}
		}
		
		this->Retain();
		
		size_t childrenCount = children.size();
		for(size_t i = 0; i< childrenCount; ++i)
		{
			children[i]->OnMaterialPropertyRemoved(propName);
		}
		
		this->Release();
	}
	
	void NMaterial::Draw(PolygonGroup * polygonGroup)
	{
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
		RenderManager::Instance()->AttachRenderData();
		
		// TODO: rethink this code
		if(polygonGroup->renderDataObject->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, 0);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, polygonGroup->indexArray);
		}
	}
	
	void NMaterial::Draw(RenderDataObject* renderData, uint16* indices, uint16 indexCount)
	{
		DVASSERT(renderData);
		
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(renderData);
		RenderManager::Instance()->AttachRenderData();
		
		// TODO: rethink this code
		if(renderData->GetIndexBufferID() != 0)
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
	
	void NMaterial::SetLight(uint32 index, Light * light)
	{
		bool changed = (light != lights[index]);
		lights[index] = light;
		
		if(changed && materialDynamicLit)
		{
			if(lights[0])
			{
				NMaterialProperty* propAmbientColor = GetMaterialProperty(NMaterial::PARAM_PROP_AMBIENT_COLOR);
				NMaterialProperty* propDiffuseColor = GetMaterialProperty(NMaterial::PARAM_PROP_DIFFUSE_COLOR);
				NMaterialProperty* propSpecularColor = GetMaterialProperty(NMaterial::PARAM_PROP_SPECULAR_COLOR);
				
				Color materialAmbientColor = (propAmbientColor) ? *(Color*) propAmbientColor->data : Color(1, 1, 1, 1);
				Color materialDiffuseColor = (propDiffuseColor) ? *(Color*) propDiffuseColor->data : Color(1, 1, 1, 1);
				Color materialSpecularColor = (propSpecularColor) ? *(Color*) propSpecularColor->data : Color(1, 1, 1, 1);
				float32 intensity = lights[0]->GetIntensity();
				
				materialAmbientColor = materialAmbientColor * lights[0]->GetAmbientColor();
				materialDiffuseColor = materialDiffuseColor * lights[0]->GetDiffuseColor();
				materialSpecularColor = materialSpecularColor * lights[0]->GetSpecularColor();
				
				SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialAmbientColor);
				SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialDiffuseColor);
				SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialSpecularColor);
				SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &intensity);
			}
			else
			{
				NMaterialProperty* propAmbientColor = GetMaterialProperty(NMaterial::PARAM_PROP_AMBIENT_COLOR);
				NMaterialProperty* propDiffuseColor = GetMaterialProperty(NMaterial::PARAM_PROP_DIFFUSE_COLOR);
				NMaterialProperty* propSpecularColor = GetMaterialProperty(NMaterial::PARAM_PROP_SPECULAR_COLOR);
				
				Color materialAmbientColor = (propAmbientColor) ? *(Color*) propAmbientColor->data : Color(1, 1, 1, 1);
				Color materialDiffuseColor = (propDiffuseColor) ? *(Color*) propDiffuseColor->data : Color(1, 1, 1, 1);
				Color materialSpecularColor = (propSpecularColor) ? *(Color*) propSpecularColor->data : Color(1, 1, 1, 1);
				float32 intensity = 0;
				
				materialAmbientColor = materialAmbientColor * Color(0, 0, 0, 0);
				materialDiffuseColor = materialDiffuseColor * Color(0, 0, 0, 0);
				materialSpecularColor = materialSpecularColor * Color(0, 0, 0, 0);
				
				SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialAmbientColor);
				SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialDiffuseColor);
				SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialSpecularColor);
				SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &intensity);
				
			}
		}
	}
	
	const RenderStateData* NMaterial::GetRenderState(const FastName& passName) const
	{
		RenderPassInstance* pass = instancePasses.at(passName);
		DVASSERT(pass);
		
		const RenderStateData* state = NULL;
		if(pass)
		{
			state = RenderManager::Instance()->GetRenderStateData(pass->renderState.stateHandle);
		}
		
		return state;
	}
	
	void NMaterial::SubclassRenderState(const FastName& passName, RenderStateData* newState)
	{
		DVASSERT(newState);
		
		RenderPassInstance* pass = instancePasses.at(passName);
		DVASSERT(pass);

		if(pass)
		{
			if(pass->dirtyState)
			{
				RenderManager::Instance()->ReleaseRenderStateData(pass->renderState.stateHandle);
			}
			
			pass->renderState.stateHandle = RenderManager::Instance()->AddRenderStateData(newState);
			pass->dirtyState = true;
		}
	}
	
	void NMaterial::SubclassRenderState(RenderStateData* newState)
	{
		for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
			it != instancePasses.end();
			++it)
		{
			SubclassRenderState(it->first, newState);
		}
	}
	
	void NMaterial::UpdateShaderWithFlags()
	{
		DVASSERT(baseTechnique);
		
		if(baseTechnique)
		{
			FastNameSet effectiveFlags(16);
			BuildEffectiveFlagSet(effectiveFlags);
			
			for(HashMap<FastName, RenderPassInstance*>::iterator it = instancePasses.begin();
				it != instancePasses.end();
				++it)
			{
				RenderPassInstance* pass = it->second;
				RenderTechniquePass* techniquePass = baseTechnique->GetPassByName(it->first);
				
				pass->renderState.shader = techniquePass->RetainShader(effectiveFlags);
				BuildActiveUniformsCacheParamsCache(pass);
			}
		}
	}
	
	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicTexturesInsp implementation
	
	size_t NMaterial::NMaterialStateDynamicTexturesInsp::MembersCount(void *object) const
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		return state->textures.size();
	}
	
	InspDesc NMaterial::NMaterialStateDynamicTexturesInsp::MemberDesc(void *object, size_t index) const
	{
		return InspDesc(MemberName(object, index));
	}
	
	const char* NMaterial::NMaterialStateDynamicTexturesInsp::MemberName(void *object, size_t index) const
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state && index >= 0 && index < state->textures.size());
		
		return state->textures.keyByIndex(index).c_str();
	}
	
	VariantType NMaterial::NMaterialStateDynamicTexturesInsp::MemberValueGet(void *object, size_t index) const
	{
		VariantType ret;
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state && index >= 0 && index < state->textures.size());
		
		TextureBucket* tex = state->textures.valueByIndex(index);
		ret.SetFilePath(tex->path);
		
		return ret;
	}
	
	void NMaterial::NMaterialStateDynamicTexturesInsp::MemberValueSet(void *object, size_t index, const VariantType &value)
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state && index >= 0 && index < state->textures.size());
		
		state->SetTexture(state->textures.keyByIndex(index), value.AsFilePath());
	}
	
	int NMaterial::NMaterialStateDynamicTexturesInsp::MemberFlags(void *object, size_t index) const
	{
		return 0;
	}
	
	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicPropertiesInsp implementation
	const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* NMaterial::NMaterialStateDynamicPropertiesInsp::FindMaterialProperties(NMaterial *state) const
	{
		static FastNameMap<PropData> staticData;
		
		staticData.clear();
		NMaterial *parent = state;
		int source = PropData::SOURCE_SELF;
		
		// properties chain data
		while(NULL != parent)
		{
			HashMap<FastName, NMaterialProperty* >::iterator it = parent->materialProperties.begin();
			HashMap<FastName, NMaterialProperty* >::iterator end = parent->materialProperties.end();
			
			for(; it != end; ++it)
			{
				if(0 == staticData.count(it->first))
				{
					PropData data;
					data.property = *it->second;
					data.source |= source;
					
					staticData.Insert(it->first, data);
					//printf("insert %s, %d\n", it->first.c_str(), source);
				}
				else
				{
					staticData[it->first].source |= source;
					//printf("update %s, %d\n", it->first.c_str(), source);
				}
			}
			
			parent = parent->GetParent();
			source = PropData::SOURCE_PARENT;
		}
		
		DVASSERT(false && "Implement for refactored new materials!");
		//VI: TODO: implement for refactored new materials
		
		/*parent = state;
		MaterialTechnique *technique = NULL;
		while(NULL != parent && NULL == technique)
		{
			technique = parent->techniqueForRenderPass[PASS_FORWARD];
			parent = parent->GetParent();
		}
		
		// shader data
		source = PropData::SOURCE_SHADER;
		if(NULL != technique)
		{
			Shader *shader = technique->GetShader();
			if(NULL != shader)
			{
				int32 uniformCount = shader->GetUniformCount();
				for(int32 i = 0; i < uniformCount; ++i)
				{
					Shader::Uniform *uniform = shader->GetUniform(i);
					if( !Shader::IsAutobindUniform(uniform->id) && // isn't auto-bind
					   uniform->type != Shader::UT_SAMPLER_2D && uniform->type != Shader::UT_SAMPLER_CUBE) // isn't texture
					{
						FastName propName = uniform->name;
						
						// redefine some shader properties names
						if(propName == NMaterial::PARAM_LIGHT_AMBIENT_COLOR) propName = NMaterial::PARAM_PROP_AMBIENT_COLOR;
						else if(propName == NMaterial::PARAM_LIGHT_DIFFUSE_COLOR) propName = NMaterial::PARAM_PROP_DIFFUSE_COLOR;
						else if(propName == NMaterial::PARAM_LIGHT_SPECULAR_COLOR) propName = NMaterial::PARAM_PROP_SPECULAR_COLOR;
						
						if(!staticData.count(propName))
						{
							PropData data;
							
							data.property.data = NULL;
							data.property.type = uniform->type;
							data.property.size = uniform->size;
							data.source |= source;
							
							staticData.Insert(propName, data);
							//printf("insert %s, %d\n", propName.c_str(), source);
						}
						else
						{
							staticData[propName].source |= source;
							//printf("update %s, %d\n", propName.c_str(), source);
						}
					}
				}
			}
		}*/
		
		return &staticData;
	}
	
	size_t NMaterial::NMaterialStateDynamicPropertiesInsp::MembersCount(void *object) const
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		
		return members->size();
	}
	
	InspDesc NMaterial::NMaterialStateDynamicPropertiesInsp::MemberDesc(void *object, size_t index) const
	{
		return InspDesc(MemberName(object, index));
	}
	
	const char* NMaterial::NMaterialStateDynamicPropertiesInsp::MemberName(void *object, size_t index) const
	{
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		DVASSERT(index < members->size());
		
		return members->keyByIndex(index).c_str();
	}
	
	int NMaterial::NMaterialStateDynamicPropertiesInsp::MemberFlags(void *object, size_t index) const
	{
		int flags = 0;
		
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		DVASSERT(index < members->size());
		
		const PropData &propData = members->valueByIndex(index);
		const FastName &propName = members->keyByIndex(index);
		
		if(propData.source & PropData::SOURCE_SELF)
		{
			flags |= I_EDIT;
		}
		
		if(propData.source & PropData::SOURCE_PARENT)
		{
			flags |= I_VIEW;
		}
		
		if(propData.source & PropData::SOURCE_SHADER)
		{
			flags |= I_SAVE;
		}
		
		return flags;
	}
	
	VariantType NMaterial::NMaterialStateDynamicPropertiesInsp::MemberValueGet(void *object, size_t index) const
	{
		VariantType ret;
		
		NMaterial *state = (NMaterial*) object;
		DVASSERT(state);
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		DVASSERT(index < members->size());
		
		FastName propName = members->keyByIndex(index);
		const NMaterialProperty *prop = &members->valueByIndex(index).property;
		
		// self or parent property
		if(NULL != prop->data)
		{
			switch(prop->type)
			{
				case Shader::UT_FLOAT:
					ret.SetFloat(*(float32*) prop->data);
					break;
					
				case Shader::UT_FLOAT_VEC2:
					ret.SetVector2(*(Vector2*) prop->data);
					break;
					
				case Shader::UT_FLOAT_VEC3:
					if(        propName == NMaterial::PARAM_LIGHT_AMBIENT_COLOR ||
					   propName == NMaterial::PARAM_LIGHT_DIFFUSE_COLOR ||
					   propName == NMaterial::PARAM_LIGHT_SPECULAR_COLOR)
					{
						float32 *color = (float32*) prop->data;
						ret.SetColor(Color(color[0], color[1], color[2], 1.0));
					}
					else
					{
						ret.SetVector3(*(Vector3*) prop->data);
					}
					break;
					
				case Shader::UT_FLOAT_VEC4:
					if(propName == NMaterial::PARAM_PROP_AMBIENT_COLOR ||
					   propName == NMaterial::PARAM_PROP_DIFFUSE_COLOR ||
					   propName == NMaterial::PARAM_PROP_SPECULAR_COLOR ||
					   propName == NMaterial::PARAM_FOG_COLOR ||
					   propName == NMaterial::PARAM_FLAT_COLOR)
					{
						float32 *color = (float32*) prop->data;
						ret.SetColor(Color(color[0], color[1], color[2], color[3]));
					}
					else
					{
						ret.SetVector4(*(Vector4*) prop->data);
					}
					break;
					
				case Shader::UT_INT:
					ret.SetInt32(*(int32*) prop->data);
					break;
					
				case Shader::UT_INT_VEC2:
				case Shader::UT_INT_VEC3:
				case Shader::UT_INT_VEC4:
					DVASSERT(false);
					//TODO: add a way to set int[]
					break;
					
				case Shader::UT_BOOL:
					ret.SetBool((*(int32*) prop->data) != 0);
					break;
					
				case Shader::UT_BOOL_VEC2:
				case Shader::UT_BOOL_VEC3:
				case Shader::UT_BOOL_VEC4:
					DVASSERT(false);
					//TODO: add a way to set bool[]
					break;
					
				case Shader::UT_FLOAT_MAT2:
					ret.SetMatrix2(*(Matrix2*) prop->data);
					break;
					
				case Shader::UT_FLOAT_MAT3:
					ret.SetMatrix3(*(Matrix3*) prop->data);
					break;
					
				case Shader::UT_FLOAT_MAT4:
					ret.SetMatrix4(*(Matrix4*) prop->data);
					break;
					
				case Shader::UT_SAMPLER_2D:
					ret.SetInt32(*(int32*) prop->data);
					break;
					
				case Shader::UT_SAMPLER_CUBE:
					ret.SetInt32(*(int32*) prop->data);
					break;
					
				default:
					DVASSERT(false);
					break;
			}
			
		}
		// shader property that is not set in self or parent properties
		else
		{
			ret = VariantType(FastName("not set"));
		}
		
		return ret;
	}
	
	void NMaterial::NMaterialStateDynamicPropertiesInsp::MemberValueSet(void *object, size_t index, const VariantType &value)
	{
		NMaterial* state = (NMaterial*) object;
		DVASSERT(index >= 0 && index < MembersCount(object));
		
		const FastNameMap<NMaterial::NMaterialStateDynamicPropertiesInsp::PropData>* members = FindMaterialProperties(state);
		DVASSERT(index < members->size());
		
		FastName propName = members->keyByIndex(index);
		const NMaterialProperty *prop = &members->valueByIndex(index).property;
		int propSize = prop->size;
		Shader::eUniformType propType = prop->type;
		
		switch(prop->type)
		{
			case Shader::UT_FLOAT:
			{
				float32 val = value.AsFloat();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_FLOAT_VEC2:
			{
				const Vector2& val = value.AsVector2();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_FLOAT_VEC3:
			{
				Vector3 val;
				
				if(        propName == NMaterial::PARAM_LIGHT_AMBIENT_COLOR ||
				   propName == NMaterial::PARAM_LIGHT_DIFFUSE_COLOR ||
				   propName == NMaterial::PARAM_LIGHT_SPECULAR_COLOR)
				{
					Color c = value.AsColor();
					val = Vector3(c.r, c.g, c.b);
				}
				else
				{
					val = value.AsVector3();
				}
				
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_FLOAT_VEC4:
			{
				Vector4 val;
				
				if(propName == NMaterial::PARAM_PROP_AMBIENT_COLOR ||
				   propName == NMaterial::PARAM_PROP_DIFFUSE_COLOR ||
				   propName == NMaterial::PARAM_PROP_SPECULAR_COLOR ||
				   propName == NMaterial::PARAM_FOG_COLOR ||
				   propName == NMaterial::PARAM_FLAT_COLOR)
				{
					Color c = value.AsColor();
					val = Vector4(c.r, c.g, c.b, c.a);
				}
				else
				{
					val = value.AsVector4();
				}
				
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_INT:
			{
				int32 val = value.AsInt32();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_INT_VEC2:
			case Shader::UT_INT_VEC3:
			case Shader::UT_INT_VEC4:
			{
				DVASSERT(false);
				//TODO: add a way to set int[]
			}
				break;
				
			case Shader::UT_BOOL:
			{
				bool val = value.AsBool();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_BOOL_VEC2:
			case Shader::UT_BOOL_VEC3:
			case Shader::UT_BOOL_VEC4:
			{
				DVASSERT(false);
				//TODO: add a way to set bool[]
			}
				break;
				
			case Shader::UT_FLOAT_MAT2:
			{
				const Matrix2& val = value.AsMatrix2();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_FLOAT_MAT3:
			{
				const Matrix3& val = value.AsMatrix3();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_FLOAT_MAT4:
			{
				const Matrix3& val = value.AsMatrix3();
				state->SetPropertyValue(propName, propType, propSize, &val);
			}
				break;
				
			case Shader::UT_SAMPLER_2D:
				//VI: samplers are set by config materials
				break;
				
			case Shader::UT_SAMPLER_CUBE:
				//VI: samplers are set by config materials
				break;
				
			default:
				DVASSERT(false);
				break;
		}
		
	}
};
