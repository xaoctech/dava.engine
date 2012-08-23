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
#ifndef __DAVAENGINE_MATERIAL_H__
#define __DAVAENGINE_MATERIAL_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"
#include "Render/RenderStateBlock.h"

namespace DAVA
{


class UberShader;
class Shader;
class Texture;    
class SceneFileV2;
class LightNode;
class PolygonGroup;
class RenderDataObject;
    
    
// TODO: move Material to Scene3D
    
class InstanceMaterialState : public BaseObject
{
    static const int32 LIGHT_NODE_MAX_COUNT = 4;
public:
    InstanceMaterialState();
    virtual ~InstanceMaterialState();
    
    void SetLight(int32 lightIndex, LightNode * lightNode);
    LightNode * GetLight(int32 lightIndex);
    
private:
    LightNode * lightNodes[LIGHT_NODE_MAX_COUNT];
};
    
    
class Material : public DataNode
{
public:
    enum eType
    {
        // Normal Materials
        MATERIAL_UNLIT_TEXTURE = 0,                 // texture
        MATERIAL_UNLIT_TEXTURE_DETAIL,              // texture * detail texture * 2.0
        MATERIAL_UNLIT_TEXTURE_DECAL,               // texture * decal 
        MATERIAL_UNLIT_TEXTURE_LIGHTMAP,            // texture * lightmap
        
        MATERIAL_VERTEX_LIT_TEXTURE,                // single texture with vertex lighting
        MATERIAL_VERTEX_LIT_DETAIL,                 // single texture * detail texture * 2.0 with vertex lighting
        MATERIAL_VERTEX_LIT_DECAL,
        MATERIAL_VERTEX_LIT_LIGHTMAP,               // vertex lit lighting + lightmaps
        
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE,
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR,     // single texture + diffuse light normal mapping
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP, // single texture + diffuse light normal mapping

		MATERIAL_VERTEX_COLOR_ALPHABLENDED,
        
        // MATERIAL_TEXTURE, 
        // MATERIAL_LIGHTMAPPED_TEXTURE,   
        // MATERIAL_VERTEX_LIGHTING,       // flag
        // MATERIAL_NORMAL_MAPPED,         // flag
        MATERIAL_TYPES_COUNT
    };
    
    static const char8 * GetTypeName(eType type);

    Material();
    virtual ~Material();
    
    
    enum eValidationResult
    {
        VALIDATE_COMPATIBLE = 1,
        VALIDATE_INCOMPATIBLE,
    };
    
    eValidationResult Validate(PolygonGroup * polygonGroup);
    
    virtual void SetScene(Scene * _scene);
   
	virtual int32 Release();
    
    void SetType(eType _type);
    
    void SetOpaque(bool _isOpaque);
    bool GetOpaque();

	void SetAlphablend(bool isAlphablend);
	bool GetAlphablend();
    
    void SetFog(bool _fogEnabled);
    bool IsFogEnabled() const;
    void SetFogDensity(float32 _fogDensity);
    float32 GetFogDensity() const;
    void SetFogColor(const Color & _fogColor);
    const Color & GetFogColor() const;
    
    
    void SetTwoSided(bool _isTwoSided);
    bool GetTwoSided();
    
    void SetAmbientColor(const Color & color);
    void SetDiffuseColor(const Color & color);
    void SetSpecularColor(const Color & color);
    void SetEmissiveColor(const Color & color);
        
    const Color & GetAmbientColor() const;
    const Color & GetDiffuseColor() const;
    const Color & GetSpecularColor() const;
    const Color & GetEmissiveColor() const;
    
    void SetShininess(float32 shininess);
    float32 GetShininess() const;

	void SetSetupLightmap(bool isSetupLightmap);
	bool GetSetupLightmap();
	void SetSetupLightmapSize(int32 setupLightmapSize);

    /**
        \brief Bind material to render system.
        Function should be used if you want to render something with this material.
     */
    //void BindMaterial();
	void PrepareRenderState();
    void Draw(PolygonGroup * group, InstanceMaterialState * state);
    
    /**
        \brief Unbind material. 
        Restore some default properties that can influence to rendering in the future.
     */
    //void UnbindMaterial();
    
    
    eType   type;

	Vector4 reflective;
	float32	reflectivity;

	Vector4 transparent;
	float	transparency; 
	float	indexOfRefraction;

	Vector2 uvOffset;
	Vector2 uvScale;

	eBlendMode blendSrc;
	eBlendMode blendDst;

    enum eTextureLevel
    {
        TEXTURE_DIFFUSE = 0,
        TEXTURE_DETAIL = 1,
        TEXTURE_NORMALMAP = 1,
        TEXTURE_DECAL = 1,
        
        TEXTURE_COUNT, 
    };
    Texture * textures[TEXTURE_COUNT];  
    String names[TEXTURE_COUNT];
    

    void Save(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile);
    void Load(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile);
    
    void SetTexture(eTextureLevel level, const String & textureName);
	inline const Texture * GetTexture(eTextureLevel level);
	inline const String & GetTextureName(eTextureLevel level);

	RenderStateBlock * GetRenderStateBlock();
    
private:
    void RebuildShader();
    
    bool    isOpaque;  
    bool    isTwoSided;

	bool	isSetupLightmap;
	int32	setupLightmapSize;
    
    float32	shininess;
    
    Color ambientColor;
	Color diffuseColor;
	Color specularColor;
	Color emissiveColor;
    
    bool    isFogEnabled;
    float32 fogDensity;
    Color   fogColor;

	bool isAlphablend;
    
    Shader  * shader;
    
    int32 uniformTexture0;
    int32 uniformTexture1;
    int32 uniformLightPosition0;
    int32 uniformMaterialLightAmbientColor;
    int32 uniformMaterialLightDiffuseColor;
    int32 uniformMaterialLightSpecularColor;
    int32 uniformMaterialSpecularShininess;
    int32 uniformLightIntensity0;
    int32 uniformLightAttenuationQ;
	int32 uniformUvOffset;
	int32 uniformUvScale;
    int32 uniformFogDensity;
    int32 uniformFogColor;

	RenderStateBlock renderStateBlock;
    
    
    /*
        TODO: Uniform array, with set of all uniforms, with one set.
     */
    
    
    
    static UberShader * uberShader;
};

const Texture * Material::GetTexture(eTextureLevel level)
{
	DVASSERT(level >= TEXTURE_COUNT);
	return textures[level];
}

inline const String & Material::GetTextureName(eTextureLevel level)
{
	DVASSERT(level >= TEXTURE_COUNT);
	return names[level];
}

};

#endif // __DAVAENGINE_MATERIAL_H__

