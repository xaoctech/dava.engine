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
#include "Render/ShaderUniformArray.h"
#include "Render/Material/NMaterialConsts.h"
#include "Render/Shader.h"
#include "Render/RenderState.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

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

class NMaterialDescriptor : public BaseObject
{
public:
    NMaterialDescriptor();
    virtual ~NMaterialDescriptor();

    uint32 GetTextureSlotByName(const String & textureName);
    uint32 GetUniformSlotByName(const String & uniformName);
    
    void SetNameForTextureSlot(uint32 slot, const String & name);
    void SetNameForUniformSlot(uint32 slot, const String & name);
    
private:
    Map<String, uint32> slotNameMap;
    Map<String, uint32> uniformNameMap;
};
    
class NMaterialProperty
{
public:
    Shader::eUniformType type;
    uint32 size;
    void * data;
};
	
    
class MaterialTechnique
{
public:
    MaterialTechnique(const FastName & _shaderName, const FastNameSet & _uniqueDefines, RenderState * _renderState);
    ~MaterialTechnique();
    
	void RecompileShader(const FastNameSet& materialDefines);

    const FastName & GetShaderName() const { return shaderName; }
    Shader * GetShader() const { return shader; }
    RenderState * GetRenderState() const { return renderState; }
    const FastNameSet & GetUniqueDefineSet() { return uniqueDefines; } 
    
protected:
    FastName shaderName;
    Shader * shader;
    RenderState * renderState;
    FastNameSet uniqueDefines;
};

class Camera;
class SerializationContext;
class NMaterial : public DataNode
{
	friend class MaterialSystem;
	
public:
    static const FastName TEXTURE_ALBEDO;
    static const FastName TEXTURE_NORMAL;
    static const FastName TEXTURE_DETAIL;
    static const FastName TEXTURE_LIGHTMAP;
	static const FastName TEXTURE_DECAL;
    
	NMaterial();
    virtual ~NMaterial();
    
    bool LoadFromFile(const String & pathname);
	void SetMaterialName(const String& name);
    
    // Work with textures and properties
    void SetTexture(const FastName & textureFastName, Texture * texture);
    Texture * GetTexture(const FastName & textureFastName) const;
    Texture * GetTexture(uint32 index);
    uint32 GetTextureCount();
    
    void SetPropertyValue(const FastName & propertyFastName, Shader::eUniformType type, uint32 size, const void * data);
	NMaterialProperty* GetMaterialProperty(const FastName & keyName);

    uint32 GetLightCount() { return lightCount; };
    void SetLight(uint32 index, Light * light) { lights[index] = light; };
    Light * GetLight(uint32 index) { return lights[index]; };

    
    void AddMaterialTechnique(const FastName & techniqueName, MaterialTechnique * materialTechnique);
    void BindMaterialTechnique(const FastName & techniqueName, Camera* camera);
    MaterialTechnique * GetTechnique(const FastName & techniqueName);
        
    void Draw(PolygonGroup * polygonGroup);
	void Draw(RenderDataObject* renderData, uint16* indices = NULL, uint16 indexCount = 0);
    
    const FastNameSet & GetRenderLayers();
	    
	void SetParent(NMaterial* material);
	void AddChild(NMaterial* material);
	void RemoveChild(NMaterial* material);
	NMaterial* CreateChild();
	
	void Rebuild(bool recursive = true);
	bool IsReady() {return ready;}
	bool IsDynamicLit() {return materialDynamicLit;}
	
	//VI: you need to manually rebuild material after defines have been changed
	//this is done in order to be able change defines serially without autorebuild
	void AddMaterialDefine(const FastName& defineName);
	void RemoveMaterialDefine(const FastName& defineName);
	
    virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
    
    // Load default render state from yaml.
    // Keep it here, by default MaterialInstance Render State should be referenced from this point.
	
private:
	
	struct TextureBucket
	{
		Texture* texture;
		size_t index;
	};
	
	struct MaterialState
	{
		FastName materialName;
		NMaterial* parent;
		FastNameSet layers;
		HashMap<FastName, MaterialTechnique *> techniqueForRenderPass; // TODO: HashMap<FastName, NMaterialInstance*> baseInstances;
		FastNameSet nativeDefines;
		HashMap<FastName, NMaterialProperty*> materialProperties;
		HashMap<FastName, TextureBucket*> textures;
		
		MaterialState()
		{
			parent = NULL;
		}
	};
	
private:
	
    void AddMaterialProperty(const String & keyName, const YamlNode * uniformNode);

	MaterialState state; //TODO: VI: this will be extended to an array for material LOD support
    
	FastNameSet inheritedDefines;
	FastNameSet effectiveLayers;
	
	Vector<NMaterial*> children;
    Vector<Texture*> texturesArray;
    Vector<FastName> textureNamesArray;
	Vector<int32> textureSlotArray;
    uint32 lightCount;
    Light * lights[8];

    //
    FastName activeTechniqueName;
    MaterialTechnique * activeTechnique;
	bool ready;
	
	bool materialDynamicLit;
	
private:
	
	void ResetParent();
	
	void OnParentChanged();
	void NotifyChildrenOnChange();
	
	void PropagateParentLayers();
	void PropagateParentDefines();
	void UnPropagateParentDefines();
	
	void BindTextures(NMaterial* curMaterial, RenderState* rs);
	void SetupPerFrameProperties(Camera* camera);
	
	void Serialize(const MaterialState& materialState, KeyedArchive * archive, SerializationContext * serializationContext);
	void Deserialize(MaterialState& materialState, KeyedArchive * archive, SerializationContext * serializationContext);

//public:
    //INTROSPECTION_EXTEND(NMaterial, DataNode,
    //     MEMBER(materialName, "Material Name", I_SAVE | I_EDIT | I_VIEW)
    //     );

};


};

#endif // __DAVAENGINE_MATERIAL_H__

