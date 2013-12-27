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
struct NMaterialTemplate;
	
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
		  
class Camera;
class SerializationContext;
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
	
	static const FastName FLAG_VERTEXFOG;
	static const FastName FLAG_TEXTURESHIFT;
	static const FastName FLAG_FLATCOLOR;
	static const FastName FLAG_LIGHTMAPONLY;
	static const FastName FLAG_TEXTUREONLY;
	static const FastName FLAG_SETUPLIGHTMAP;
	
	enum eMaterialType
	{
		MATERIALTYPE_NONE = 0,
		MATERIALTYPE_MATERIAL = 1,
		MATERIALTYPE_INSTANCE = 2
	};
	
	enum eFlagValue
	{
		FlagOff = 0,
		FlagOn = 1,
		FlagOffOverride = 2,
		FlagOnOverride = 3
	};
	
public:
	
	NMaterial();
	
	inline NMaterial* GetParent() const {return parent;}
	
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
	
	void SetFlag(const FastName& flag, eFlagValue flagValue);
	int32 GetFlagValue(const FastName& flag) const;
    
	void BindMaterialTechnique(const FastName & passName, Camera* camera);
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
	
	const RenderStateData* GetRenderState(const FastName& passName) const;
	void SubclassRenderState(const FastName& passName, RenderStateData* newState);
	void SubclassRenderState(RenderStateData* newState);
	
protected:
	
	class GenericPropertyManager
	{
	public:
		static void Init(NMaterialProperty* prop);
		static void Release(NMaterialProperty* prop);
		static NMaterialProperty* Clone(NMaterialProperty* prop);
	};
		
	typedef NManagedMaterialProperty<GenericPropertyManager> GenericMaterialProperty;

	struct TextureBucket
	{
		TextureBucket() : texture(NULL)
		{ }

		Texture* texture; //VI: can be NULL
		FilePath path;
	};
		
	struct UniformCacheEntry
	{
		UniformCacheEntry() : uniform(NULL), prop(NULL)
		{ }

		Shader::Uniform* uniform;
		int32 index;
		NMaterialProperty* prop;
	};
		
	struct RenderPassInstance
	{
		RenderState renderState;
		
		bool dirtyState;
		bool texturesDirty;
		
		HashMap<FastName, int32> textureIndexMap;
		Vector<UniformCacheEntry> activeUniformsCache;
		
		UniformCacheEntry* activeUniformsCachePtr;
		size_t activeUniformsCacheSize;
		
		RenderPassInstance() : textureIndexMap(8)
		{
			
		}
	};
	
protected:
		
	FastName materialName;
	eMaterialType materialType;
	NMaterialKey materialKey;

	HashMap<FastName, NMaterialProperty*> materialProperties;
	
	HashMap<FastName, TextureBucket*> textures;
	
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

	RenderTechnique* baseTechnique;
	HashMap<FastName, RenderPassInstance*> instancePasses;
	
	RenderPassInstance* activePassInstance;
	RenderTechniquePass* activeRenderPass;
	FastName activePassName;
	
	FastName currentQuality;
			
    IlluminationParams * illuminationParams;
	
	const NMaterialTemplate* materialTemplate;
	
	//VI: material flags alter per-instance shader. For example, adding fog, texture animation etc
	HashMap<FastName, int32> materialSetFlags; //VI: flags set in the current material only
	
protected:
	
	virtual ~NMaterial();
	
	inline void SetMaterialType(eMaterialType matType) {materialType = matType;}
	inline void SetMaterialKey(NMaterialKey key) {materialKey = key;}
	void SetMaterialTemplate(const NMaterialTemplate* matTemplate, const FastName& defaultQuality);
	
	void BuildEffectiveFlagSet(FastNameSet& effectiveFlagSet);
	void BuildEffectiveFlagSet(HashMap<FastName, int32>& effectiveFlagSet);
		
	void ReleaseInstancePasses();
	
	void UpdateMaterialTemplate();
	void UpdateRenderPass(const FastName& passName,
						  const FastNameSet& instanceDefines,
						  RenderTechniquePass* pass);
	void BuildTextureParamsCache(RenderPassInstance* passInstance);
	void BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance);
	TextureBucket* GetTextureBucketRecursive(const FastName& textureFastName) const;
	
	void LoadActiveTextures();
	void CleanupUnusedTextures();
	Texture* GetOrLoadTextureRecursive(const FastName& textureName);
	bool IsTextureActive(const FastName& textureName) const;
	void SetTexturesDirty();
	void PrepareTextureState(RenderPassInstance* passInstance);
	void UpdateShaderWithFlags();
	
	void SetupPerFrameProperties(Camera* camera);
	void BindMaterialTextures(RenderPassInstance* passInstance);
	void BindMaterialProperties(RenderPassInstance* passInstance);
		
protected:
	
	void OnParentChanged(NMaterial* newParent);
	void OnMaterialTemplateChanged();
	void OnParentFlagsChanged();
	void OnInstanceQualityChanged();
	
	void OnMaterialPropertyAdded(const FastName& propName, NMaterialProperty* prop);
	void OnMaterialPropertyRemoved(const FastName& propName);
	
public:
	
	class NMaterialStateDynamicTexturesInsp : public InspInfoDynamic
	{
	public:
		size_t MembersCount(void *object) const;
		InspDesc MemberDesc(void *object, size_t index) const;
		const char* MemberName(void *object, size_t index) const;
		int MemberFlags(void *object, size_t index) const;
		VariantType MemberValueGet(void *object, size_t index) const;
		void MemberValueSet(void *object, size_t index, const VariantType &value);
	};
	
	class NMaterialStateDynamicPropertiesInsp : public InspInfoDynamic
	{
	public:
		size_t MembersCount(void *object) const;
		InspDesc MemberDesc(void *object, size_t index) const;
		const char* MemberName(void *object, size_t index) const;
		int MemberFlags(void *object, size_t index) const;
		VariantType MemberValueGet(void *object, size_t index) const;
		void MemberValueSet(void *object, size_t index, const VariantType &value);
		
	protected:
		struct PropData
		{
			enum PropSource
			{
				SOURCE_UNKNOWN = 0x0,
				SOURCE_SELF = 0x1,
				SOURCE_PARENT = 0x2,
				SOURCE_SHADER = 0x4
			};
			
			PropData() : source(SOURCE_UNKNOWN)
			{ }
			
			int source;
			NMaterialProperty property;
		};
		
		const FastNameMap<PropData>* FindMaterialProperties(NMaterial *state) const;
	};
	
public:
	
	INTROSPECTION(NMaterial,
				  DYNAMIC(textures, "Material textures", new NMaterialStateDynamicTexturesInsp(), I_SAVE | I_EDIT | I_VIEW)
				  DYNAMIC(materialProperties, "Material properties", new NMaterialStateDynamicPropertiesInsp(), I_SAVE | I_EDIT | I_VIEW)
				  );

};

};

#endif // __DAVAENGINE_MATERIAL_H__

