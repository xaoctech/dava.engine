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

namespace DAVA
{

class UberShader;
class Texture;    
class SceneFileV2;
class Light;
class PolygonGroup;
class RenderDataObject;
class RenderState;
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

/*
    Sorting should be done by NMaterialInstance shader, because it can be changed by material
 */
class NMaterialInstance : public BaseObject
{
public:
    NMaterialInstance();
    virtual ~NMaterialInstance();
    
    void Save(KeyedArchive * archive, SceneFileV2 *sceneFile);
	void Load(KeyedArchive * archive, SceneFileV2 *sceneFile);

    void PrepareInstanceForShader(Shader * shader);
    RenderState * GetRenderState() { return &renderState; };

    // Functions
    void SetUniformData(uint32 uniformIndex, void * data, uint32 size);
    void UpdateUniforms();
    void BindUniforms();

    // Helper functions
    uint32 GetLightCount() { return lightCount; };
    void SetLight(uint32 index, Light * light) { lights[index] = light; };
    Light * GetLight(uint32 index) { return lights[index]; };
    
    void SetTwoSided(bool isTwoSided);
    bool IsTwoSided();
    
    void SetLightmap(Texture * texture, const FilePath & lightmapName);
    void SetUVOffsetScale(const Vector2 & uvOffset, const Vector2 uvScale);
    
    Texture * GetLightmap() const;
    String GetLightmapName() const;

    
    // Prepare and draw functions
    void PrepareRenderState();
    void Draw(PolygonGroup * polygonGroup);
    
    bool IsExportOwnerLayerEnabled() const;
    void SetExportOwnerLayer(const bool & isEnabled);
    FastName GetOwnerLayerName() const;
    void SetOwnerLayerName(const FastName & fastname);
    
    NMaterialInstance * Clone();
    
private:
    
    static const uint32 SKIP_UNIFORM = 1 << 0;
    
    
    struct UniformInfo
    {
        uint32  flags: 8;
        uint32  arraySize : 8;
        uint32  shift : 16;
    };
    
    
    uint32 uniformCount;
    UniformInfo * uniforms;
    uint8 * uniformData;
    uint32 lightCount;
    Light * lights[8];
    RenderState renderState;
    Shader * shader;
    friend class NMaterial;
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
    MaterialTechnique(const FastName & _shaderName, FastNameSet & _uniqueDefines, RenderState * _renderState);
    ~MaterialTechnique();
    
    void RecompileShader();

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
 
class NMaterial : public DataNode
{
public:
    NMaterial();
    virtual ~NMaterial();
    
    bool LoadFromFile(const String & pathname);
    
    void AddMaterialTechnique(FastName & techniqueName, MaterialTechnique * materialTechnique);
    void BindMaterialTechnique(const FastName & techniqueName);
    MaterialTechnique * GetTechnique(const FastName & techniqueName);
    void Draw(PolygonGroup * polygonGroup);


    const FastNameSet & GetRenderLayers() { return layers; };
    
    NMaterial * CreateChildMaterial();
    
    // Load default render state from yaml.
    // Keep it here, by default MaterialInstance Render State should be referenced from this point.
    
private:
    NMaterial * parent;
    FastNameSet layers;
    HashMap<FastName, NMaterialProperty*> materialProperties;
    HashMap<FastName, MaterialTechnique *> techniqueForRenderPass; // TODO: HashMap<FastName, NMaterialInstance*> baseInstances;

    //
    FastName activeTechniqueName;
    MaterialTechnique * activeTechnique;

};


};

#endif // __DAVAENGINE_MATERIAL_H__

