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

namespace DAVA
{


class UberShader;
class Shader;
class Texture;    
class SceneFileV2;
class LightNode;
class PolygonGroup;
    
    
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
        MATERIAL_VERTEX_LIT_DETAIL,         // single texture * detail texture * 2.0 with vertex lighting
        MATERIAL_VERTEX_LIT_DECAL,
        
        MATERIAL_NORMAL_MAPPED_DIFFUSE,     // single texture + diffuse light normal mapping
        MATERIAL_NORMAL_MAPPED_SPECULAR,    // single texture + diffuse + specular normal mapping
        
        // MATERIAL_TEXTURE, 
        // MATERIAL_LIGHTMAPPED_TEXTURE,   
        // MATERIAL_VERTEX_LIGHTING,       // flag
        // MATERIAL_NORMAL_MAPPED,         // flag
        MATERIAL_TYPES_COUNT
    };

    Material(Scene * _scene = 0);
    virtual ~Material();
    
    virtual void SetScene(Scene * _scene);
   
	virtual int32 Release();
    static const char * GetTypeName();
    
    void SetType(eType _type);
    void SetOpaque(bool isOpaque);
    bool GetOpaque();

    void SetTwoSided(bool isTwoSided);
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
        
    void Draw(PolygonGroup * group, InstanceMaterialState * state);
    
    eType   type;

	Vector4 reflective;
	float32	reflectivity;

	Vector4 transparent;
	float	transparency; 
	float	indexOfRefraction;

    enum
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
    
    
private:
    void RebuildShader();
    
    bool    isOpaque;  
    bool    isTwoSided;
    
    float32	shininess;
    
    Color ambientColor;
	Color diffuseColor;
	Color specularColor;
	Color emissiveColor;
    
    Shader  * shader;
    
    int32 uniformTexture0;
    int32 uniformTexture1;
    int32 uniformLightPosition0;
    int32 uniformMaterialDiffuseColor;
    int32 uniformMaterialSpecularColor;
    
    static UberShader * uberShader;
};

};

#endif // __DAVAENGINE_MATERIAL_H__

