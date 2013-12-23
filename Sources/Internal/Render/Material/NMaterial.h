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

#ifndef __DAVAENGINE_NMATERIAL_H__
#define __DAVAENGINE_NMATERIAL_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Scene3D/DataNode.h"
#include "Render/RenderState.h"
#include "Render/Material/NMaterialConsts.h"
#include "Render/Shader.h"
#include "Render/RenderState.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Material/RenderTechnique.h"

namespace DAVA
{

class UberShader;
class Texture;    
class SceneFileV2;
class Light;
class PolygonGroup;
class RenderDataObject;
class Light;
class MaterialCompiler;
class MaterialGraph;
	
class NMaterial;
class NMaterialTemplate;
	
struct IlluminationParams : public InspBase
{
    static const int32 LIGHTMAP_SIZE_DEFAULT = 128;
    
    bool isUsed;
    bool castShadow;
    bool receiveShadow;
    int32 lightmapSize;

    void SetDefaultParams() 
    {
        isUsed = castShadow = receiveShadow = true;
        lightmapSize = LIGHTMAP_SIZE_DEFAULT;
    }

    INTROSPECTION(IlluminationParams, 
        MEMBER(isUsed, "Use Illumination", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(castShadow, "Cast Shadow", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(receiveShadow, "Receive Shadow", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(lightmapSize, "Lightmap Size", I_SAVE | I_VIEW | I_EDIT)
        );
};

class NMaterialProperty
{
public:
    Shader::eUniformType type;
    uint8 size;
    void* data;
	
	virtual ~NMaterialProperty()
	{
	}
	
	virtual NMaterialProperty* Clone()
	{
		return NULL;
	}
};
	
template<typename MEMMANAGER>
class NManagedMaterialProperty : public NMaterialProperty
{
	
public:
	
	NManagedMaterialProperty()
	{
		MEMMANAGER::Init(this);
	}

	virtual ~NManagedMaterialProperty()
	{
		MEMMANAGER::Release(this);
	}
	
	virtual NMaterialProperty* Clone()
	{
		return MEMMANAGER::Clone(this);
	}
};
		  
/*class MaterialTechnique
{
		
public:
	
    MaterialTechnique(const FastName & _shaderName, const FastNameSet & _uniqueDefines, RenderState * _renderState);
    ~MaterialTechnique();
    
	void RecompileShader(const FastNameSet& materialDefines);

    const FastName & GetShaderName() const { return shaderName; }
    inline Shader * GetShader() const { return shader; }
    inline RenderState * GetRenderState() const { return renderState; }
    const FastNameSet & GetUniqueDefineSet() { return uniqueDefines; }
	    
protected:
    FastName shaderName;
    Shader * shader;
    RenderState * renderState;
    FastNameSet uniqueDefines;
	};
*/
	
class Camera;
class SerializationContext;
class MaterialSystem;
class NMaterial : public DataNode
{
	friend class MaterialSystem;
	friend class MaterialCompiler;
	
public:
	
	typedef uint64 NMaterialKey;

    static const FastName TEXTURE_ALBEDO;
    static const FastName TEXTURE_NORMAL;
    static const FastName TEXTURE_DETAIL;
    static const FastName TEXTURE_LIGHTMAP;
	static const FastName TEXTURE_DECAL;
	
	static const FastName PARAM_LIGHT_POSITION0;
	static const FastName PARAM_PROP_AMBIENT_COLOR;
	static const FastName PARAM_PROP_DIFFUSE_COLOR;
	static const FastName PARAM_PROP_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_AMBIENT_COLOR;
	static const FastName PARAM_LIGHT_DIFFUSE_COLOR;
	static const FastName PARAM_LIGHT_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_INTENSITY0;
	static const FastName PARAM_MATERIAL_SPECULAR_SHININESS;
	static const FastName PARAM_FOG_COLOR;
	static const FastName PARAM_FOG_DENSITY;
	static const FastName PARAM_FLAT_COLOR;
	static const FastName PARAM_TEXTURE0_SHIFT;
	static const FastName PARAM_UV_OFFSET;
	static const FastName PARAM_UV_SCALE;
	
	enum eMaterialType
	{
		MATERIALTYPE_NONE = 0,
		MATERIALTYPE_MATERIAL = 1,
		MATERIALTYPE_INSTANCE = 2
	};
	
public:
	
	NMaterial();
	
	inline NMaterial* GetParent() {return parent;}
	
	void AddChild(NMaterial* material);
	void RemoveChild(NMaterial* material);
    int32 GetChildrenCount() const;
    NMaterial *GetChild(int32 index) const;
		
	//{TODO: these should be removed and changed to a generic system
	//setting properties via special setters
    uint32 GetLightCount() { return lightCount; };
    void SetLight(uint32 index, Light * light);
    Light * GetLight(uint32 index) { return lights[index]; };
	inline bool IsDynamicLit() {return materialDynamicLit;}
	//}END TODO
	
    void Draw(PolygonGroup * polygonGroup);
	void Draw(RenderDataObject* renderData, uint16* indices = NULL, uint16 indexCount = 0);
	
	void SetFlag(const FastName& flag, bool flagValue);
	void SetBlockFlag(const FastName& flag, bool blockState);
	bool GetFlagValue(const FastName& flag);
	bool IsFlagBlocked(const FastName& flag);
    
	void BindMaterialTechnique(const FastName & techniqueName, Camera* camera);
	inline uint32 GetRequiredVertexFormat() {return requiredVertexFormat;}
		
	virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
	
	bool SwitchQuality(const FastName& stateName);
	
	NMaterial* Clone();
	NMaterial* Clone(const String& newName);
		
    IlluminationParams * GetIlluminationParams();
    void ReleaseIlluminationParams();
	
	// Work with textures and properties
    void SetTexture(const FastName& textureFastName, const FilePath& texturePath);
	void SetTexture(const FastName& textureFastName, Texture* texture);
    Texture * GetTexture(const FastName& textureFastName) const;
	const FilePath& GetTexturePath(const FastName& textureFastName) const;
    Texture * GetTexture(int32 index) const;
	const FilePath& GetTexturePath(int32 index) const;
	const FastName& GetTextureName(int32 index) const;
    uint32 GetTextureCount() const;
    
    void SetPropertyValue(const FastName & keyName,
						  Shader::eUniformType type,
						  uint32 size,
						  const void * data);
	NMaterialProperty* GetPropertyValue(const FastName & keyName) const;
	NMaterialProperty* GetMaterialProperty(const FastName & keyName) const;
	void RemoveMaterialProperty(const FastName & keyName);
	
	void SetMaterialName(const String& name);
	inline const FastName& GetMaterialName() const {return materialName;}
	
	inline eMaterialType GetMaterialType() const {return materialType;}
	
	inline NMaterialKey GetMaterialKey() {return materialKey;}
	
	const FastNameSet& GetRenderLayers();
	
protected:
	
	class GenericPropertyManager
	{
	public:
		static void Init(NMaterialProperty* prop);
		static void Release(NMaterialProperty* prop);
		static NMaterialProperty* Clone(NMaterialProperty* prop);
	};
	
	class UniformPropertyManager
	{
	public:
		static void Init(NMaterialProperty* prop);
		static void Release(NMaterialProperty* prop);
		static NMaterialProperty* Clone(NMaterialProperty* prop);
	};
	
	typedef NManagedMaterialProperty<GenericPropertyManager> GenericMaterialProperty;
	typedef NManagedMaterialProperty<UniformPropertyManager> UniformMaterialProperty;

	struct TextureBucket
	{
		Texture* texture; //VI: can be NULL
		FilePath path;
	};
	
	struct TextureParamCacheEntry
	{
		FastName textureName;
		int32 slot;
		Texture* tx;
	};
	
	struct UniformCacheEntry
	{
		Shader::Uniform* uniform;
		int32 index;
		void* propData;
	};
		
	struct RenderPassInstance
	{
		Shader* shader;
		UniqueHandle renderState;
		
		bool dirtyState;
		
		Vector<TextureParamCacheEntry> textureParamsCache;
		Vector<UniformCacheEntry> activeUniformsCache;
		
		TextureParamCacheEntry* textureParamsCachePtr;
		UniformCacheEntry* activeUniformsCachePtr;
		size_t textureParamsCacheSize;
		size_t activeUniformsCacheSize;
		
		bool ContainsTexture(const FastName& name)
		{
			for(size_t i = 0; i < textureParamsCacheSize; ++i)
			{
				if(textureParamsCachePtr[i].textureName == name)
				{
					return true;
				}
			}
			
			return false;
		}
	};
	
protected:
		
	FastName materialName;
	eMaterialType materialType;
	NMaterialKey materialKey;

	HashMap<FastName, NMaterialProperty*> materialProperties;
	
	HashMap<FastName, TextureBucket*> textures;
	UniqueHandle textureStateHandle;
	bool texturesDirty;
	
	NMaterial* parent;
	Vector<NMaterial*> children;
	
	//TODO: fill it from configuration
	uint32 requiredVertexFormat;

 	//{TODO: these should be removed and changed to a generic system
	//setting properties via special setters
    uint32 lightCount;
    Light * lights[8];
	bool materialDynamicLit;
	//}END TODO

    //FastName activeTechniqueName;
    //MaterialTechnique * activeTechnique;
	
	RenderTechnique* baseTechnique;
	HashMap<FastName, RenderPassInstance*> instancePasses;
	
	RenderPassInstance* activePassInstance;
	RenderTechniquePass* activeRenderPass;
	
	bool ready;
	
	FastName currentQuality;
			
    IlluminationParams * illuminationParams;
	
	const NMaterialTemplate* materialTemplate;
	
	Vector<uint8> uniformDataStorage;
	
	//VI: material flags alter per-instance shader. For example, adding fog, texture animation etc
	FastNameSet materialSetFlags; //VI: flags set in the current material only
	FastNameSet materialBlockedFlags; //VI: flags blocked in the current material only
	
protected:
	
	virtual ~NMaterial();
	
	inline void SetMaterialType(eMaterialType matType) {materialType = matType;}
	inline void SetMaterialKey(NMaterialKey key) {materialKey = key;}
	void SetMaterialTemplate(const NMaterialTemplate* matTemplate);
	
	void BuildEffectiveFlagSet(FastNameSet effectiveFlagSet);
	
	void DeserializeFastNameSet(const KeyedArchive* srcArchive,
								FastNameSet& targetSet);
	void SerializeFastNameSet(const FastNameSet& srcSet,
							  KeyedArchive* targetArchive);
	
	void ReleaseInstancePasses();
	
	void UpdateMaterialTemplate();
	void UpdateRenderPass(const FastName& passName,
						  const FastNameSet& instanceDefines,
						  RenderTechniquePass* pass);
	void BuildTextureParamsCache(RenderPassInstance* passInstance);
	void BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance);
	TextureBucket* GetTextureBucketRecursive(const FastName& textureFastName) const;
		
protected:
	
	void OnParentChanged(NMaterial* newParent);
	void OnMaterialTemplateChanged();
	void OnParentFlagsChanged();
	void OnInstanceQualityChanged();
	
public:
	
	class NMaterialStateDynamicTexturesInsp : public InspInfoDynamic
	{
	public:
		int MembersCount(void *object) const;
		InspDesc MemberDesc(void *object, int index) const;
		const char* MemberName(void *object, int index) const;
		VariantType MemberValueGet(void *object, int index) const;
		void MemberValueSet(void *object, int index, const VariantType &value);
	};
	
	class NMaterialStateDynamicPropertiesInsp : public InspInfoDynamic
	{
	private:
		
		void MemberValueSetInternal(NMaterial* state, int index, const VariantType &value);
		
	public:
		int MembersCount(void *object) const;
		InspDesc MemberDesc(void *object, int index) const;
		const char* MemberName(void *object, int index) const;
		VariantType MemberValueGet(void *object, int index) const;
		void MemberValueSet(void *object, int index, const VariantType &value);
	};
	
public:
	
	INTROSPECTION(NMaterial,
				  DYNAMIC(textures, "Material textures", new NMaterialStateDynamicTexturesInsp(), I_SAVE | I_EDIT | I_VIEW)
				  DYNAMIC(materialProperties, "Material properties", new NMaterialStateDynamicPropertiesInsp(), I_SAVE | I_EDIT | I_VIEW)
				  );

};

};

#endif // __DAVAENGINE_MATERIAL_H__

