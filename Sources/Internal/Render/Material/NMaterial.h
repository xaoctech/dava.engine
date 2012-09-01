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
#include "Scene3D/DataNode.h"

namespace DAVA
{


class UberShader;
class Shader;
class Texture;    
class SceneFileV2;
class LightNode;
class PolygonGroup;
class RenderDataObject;
    

    
/*
    Thoughts on smart material system we need

    // Inputs
    // Streams
    Declarative (to know which streams is required for material)
    // Uniforms
    Should be connected using some smart mechanism to avoid writing code. Pin Connectors. 

    // Samplers
    Again pin connectors. 

    // Outputs
    Define what output this given material. How many channels, how many render targets and so on. 

    // UberShader that lies inside the material
 */
    
class MaterialCompiler
{
public:
    
    NMaterial * Compile(MaterialGraphNode * graphMaterial);
    
    
};
     
class GraphMaterial
{
public:
    // 
    void GetTextureSlotCount();
    const char * GetTextureSlotName(uint32 index);

    //
    // Underlying material 
    //
    
};
    
class ExpressionInput
{
    uint32 mask;        // first 4 bits declare available channels (xyzw | rgba)
};
    
class ExpressionOutput
{
    uint32 mask;        // 
};
    
class MaterialGraphNode
{
public:
    MaterialGraphNode();
    ~MaterialGraphNode();
    
    uint32 GetInputCount();
    ExpressionInput * GetInput(uint32 index);
    
    uint32 GetOutputCount();
    ExpressionOutput * GetOutput(uint32 output);
    
    
};
    
class MaterialGraphConnector
{
public:
    ExpressionInput * input;
    ExpressionOutput * output;
};
    
class TextureSamplerNode : public MaterialGraphNode
{
public:
    
};
     
class MaterialConstBufferNames
{
public:


};
    
// This is packed structure. To work with this structure in editor you need it with MaterialConstBufferNames
// 
class MaterialConstBuffer
{
public:
    struct UniformDataInput
    {   
        uint32 uniformIndex;     
    };
    // structure of data
    // uint32 type;
    // uint32 size;
    // uint8 data[size]; // aligned to 32
    // uint32 next;      // shift to next ??? is it required??? 
    uint32 * data;
};
 
 
class NMaterial
{
public:
    NMaterial(Shader * shader);
    
    // Give pointer to texture
    void SetTexture(uint32 level, Texture * texture);

    // Give pointer to data where I should take them
    // Problems: Dead pointers??? Generally it should be data within the EntitySystem data storage to check correctness of the data
    // Multithreading: ??? Potentially it shoudn't be a problem, but who knows
    //void SetUniformInput(uint32 uniformIndex, MaterialConstantBuffer * buffer, eUniformType type, uint32 count, uint32 shift);

    void SetMaterialConstBuffer(MaterialConstBuffer * buffer);

    // Render State Block, we should find a good way to pass render state blocks. Probably we should cache render state blocks??? 
    void SetRenderState(RenderStateBlock * block);

private: 
    RenderStateBlock * block;
    Texture * textures[MAX_TEXTURE_UNIT_COUNT];
    MaterialConstBuffer * buffer;
    struct UniformDataInput
    {
        eUniformType type;          // first ready of uniform type should cache the whole structure
        uint32 uniformIndex: 28;    // can be merged with count to save bytes
        uint32 count: 4;
        uint32 shift;
    };
    Vector<UniformDataInput> uniforms;
};
    
//class Material : public DataNode
//{
//public:
//    enum eType
//    {
//        // Normal Materials
//        MATERIAL_UNLIT_TEXTURE = 0,                 // texture
//        MATERIAL_UNLIT_TEXTURE_DETAIL,              // texture * detail texture * 2.0
//        MATERIAL_UNLIT_TEXTURE_DECAL,               // texture * decal 
//        MATERIAL_UNLIT_TEXTURE_LIGHTMAP,            // texture * lightmap
//        
//        MATERIAL_VERTEX_LIT_TEXTURE,                // single texture with vertex lighting
//        MATERIAL_VERTEX_LIT_DETAIL,                 // single texture * detail texture * 2.0 with vertex lighting
//        MATERIAL_VERTEX_LIT_DECAL,
//        MATERIAL_VERTEX_LIT_LIGHTMAP,               // vertex lit lighting + lightmaps
//        
//        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE,
//        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR,     // single texture + diffuse light normal mapping
//        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP, // single texture + diffuse light normal mapping
//
//		MATERIAL_VERTEX_COLOR_ALPHABLENDED,
//        MATERIAL_LANDSCAPE, 
//        
//        // MATERIAL_TEXTURE, 
//        // MATERIAL_LIGHTMAPPED_TEXTURE,   
//        // MATERIAL_VERTEX_LIGHTING,       // flag
//        // MATERIAL_NORMAL_MAPPED,         // flag
//        MATERIAL_TYPES_COUNT
//    };
//    
//    static const char8 * GetTypeName(eType type);
//
//    Material();
//    virtual ~Material();
//    
//    
//    enum eValidationResult
//    {
//        VALIDATE_COMPATIBLE = 1,
//        VALIDATE_INCOMPATIBLE,
//    };
//    
//    eValidationResult Validate(PolygonGroup * polygonGroup);
//    
//    virtual void SetScene(Scene * _scene);
//   
//	virtual int32 Release();
//   
//    void SetType(eType _type);
//   
//    
//    void SetOpaque(bool _isOpaque);
//    bool GetOpaque();
//
//	void SetAlphablend(bool isAlphablend);
//	bool GetAlphablend();
//    
//    void SetFog(bool _fogEnabled);
//    bool IsFogEnabled() const;
//    void SetFogDensity(float32 _fogDensity);
//    float32 GetFogDensity() const;
//    void SetFogColor(const Color & _fogColor);
//    const Color & GetFogColor() const;
//    
//    
//    void SetTwoSided(bool _isTwoSided);
//    bool GetTwoSided();
//    
//    void SetAmbientColor(const Color & color);
//    void SetDiffuseColor(const Color & color);
//    void SetSpecularColor(const Color & color);
//    void SetEmissiveColor(const Color & color);
//        
//    const Color & GetAmbientColor() const;
//    const Color & GetDiffuseColor() const;
//    const Color & GetSpecularColor() const;
//    const Color & GetEmissiveColor() const;
//    
//    void SetShininess(float32 shininess);
//    float32 GetShininess() const;
//
//	void SetSetupLightmap(bool isSetupLightmap);
//	bool GetSetupLightmap();
//	void SetSetupLightmapSize(int32 setupLightmapSize);
//
//    /**
//        \brief Bind material to render system.
//        Function should be used if you want to render something with this material.
//     */
//    //void BindMaterial();
//	void PrepareRenderState();
//    void Draw(PolygonGroup * group, InstanceMaterialState * state);
//    
//    /**
//        \brief Unbind material. 
//        Restore some default properties that can influence to rendering in the future.
//     */
//    //void UnbindMaterial();
//    
//    
//    eType   type;
//
//	Vector4 reflective;
//	float32	reflectivity;
//
//	Vector4 transparent;
//	float	transparency; 
//	float	indexOfRefraction;
//
//	Vector2 uvOffset;
//	Vector2 uvScale;
//
//	eBlendMode blendSrc;
//	eBlendMode blendDst;
//
//
//    /*
//        default materials slots
//        
//        diffuse map
//        detail map
//        light map
//        decal map
//        normal map
//        specular map
//        height map
//        
//        // all these maps can be separate textures and also can be channels of textures
//     
//    */
//    
//    enum eTextureLevel
//    {
//        TEXTURE_DIFFUSE = 0,
//        TEXTURE_DETAIL = 1,
//        TEXTURE_NORMALMAP = 1,
//        TEXTURE_DECAL = 1,
//        
//        TEXTURE_COUNT = 8, 
//    };
//    Texture * textures[TEXTURE_COUNT];  
//    String names[TEXTURE_COUNT];
//    
//
//    void Save(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile);
//    void Load(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile);
//    
//    /*
//        
//     */
//    void GetTextureSlotCount(); 
//    const char * GetTextureSlotName(uint32 index);
//
//    void SetTexture(uint32 level, const String & textureName);
//	inline const Texture * GetTexture(uint32 level);
//	inline const String & GetTextureName(uint32 level);
//    
//private:
//    void RebuildShader();
//    
//    bool    isOpaque;  
//    bool    isTwoSided;
//
//	bool	isSetupLightmap;
//	int32	setupLightmapSize;
//    
//    float32	shininess;
//    
//    Color ambientColor;
//	Color diffuseColor;
//	Color specularColor;
//	Color emissiveColor;
//    
//    bool    isFogEnabled;
//    float32 fogDensity;
//    Color   fogColor;
//
//	bool isAlphablend;
//    
//    Shader  * shader;
//    
//    int32 uniformTexture0;
//    int32 uniformTexture1;
//    int32 uniformLightPosition0;
//    int32 uniformMaterialLightAmbientColor;
//    int32 uniformMaterialLightDiffuseColor;
//    int32 uniformMaterialLightSpecularColor;
//    int32 uniformMaterialSpecularShininess;
//    int32 uniformLightIntensity0;
//    int32 uniformLightAttenuationQ;
//	int32 uniformUvOffset;
//	int32 uniformUvScale;
//    int32 uniformFogDensity;
//    int32 uniformFogColor;
//    
//    
//    /*
//        TODO: Uniform array, with set of all uniforms, with one set.
//     */
//    
//    // Landscape uniforms
//    int32   uniformCameraPosition;
//    int32   uniformTextures[TEXTURE_COUNT];
//    int32   uniformTextureTiling[TEXTURE_COUNT];
//    Vector2 textureTiling[TEXTURE_COUNT];
//    
//    
//    static UberShader * uberShader;
//};
//
//const Texture * Material::GetTexture(uint32 level)
//{
//	DVASSERT(level >= TEXTURE_COUNT);
//	return textures[level];
//}
//
//inline const String & Material::GetTextureName(uint32 level)
//{
//	DVASSERT(level >= TEXTURE_COUNT);
//	return names[level];
//}
    
    

};

#endif // __DAVAENGINE_MATERIAL_H__

