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
#include "Render/Material.h"
#include "Render/UberShader.h"
#include "Render/Texture.h"
#include "FileSystem/KeyedArchive.h"
#include "Utils/StringFormat.h"
#include "Render/Shader.h"
#include "Render/Image.h"
#include "Render/3D/PolygonGroup.h"
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Scene3D/LightNode.h"

namespace DAVA 
{
    
    
InstanceMaterialState::InstanceMaterialState()
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        lightNodes[k] = 0;
}

InstanceMaterialState::~InstanceMaterialState()
{
    for (int32 k = 0; k < LIGHT_NODE_MAX_COUNT; ++k)
        SafeRelease(lightNodes[k]);
}

void InstanceMaterialState::SetLight(int32 lightIndex, LightNode * lightNode)
{ 
    SafeRelease(lightNodes[lightIndex]);
    lightNodes[lightIndex] = SafeRetain(lightNode); 
}

LightNode * InstanceMaterialState::GetLight(int32 lightIndex) 
{ 
    return lightNodes[lightIndex]; 
}

    
REGISTER_CLASS(Material);
    
UberShader * Material::uberShader = 0;
    
const char8 * Material::GetTypeName(eType format)
{
    switch(format)
    {
        case MATERIAL_UNLIT_TEXTURE:
            return "UNLIT_TEXTURE";
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            return "UNLIT_TEXTURE_DETAIL";
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            return "UNLIT_TEXTURE_DECAL";
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
            return "UNLIT_TEXTURE_LIGHTMAP";
        case MATERIAL_VERTEX_LIT_TEXTURE:
            return "VERTEX_LIT_TEXTURE";
        case MATERIAL_VERTEX_LIT_DETAIL:
            return "VERTEX_LIT_DETAIL";
        case MATERIAL_VERTEX_LIT_DECAL:
            return "VERTEX_LIT_DECAL";
        case MATERIAL_VERTEX_LIT_LIGHTMAP:
            return "VERTEX_LIT_LIGHTMAP";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
            return "PIXEL_LIT_NORMAL_DIFFUSE";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
            return "PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR";
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            return "PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP";
        default:
            break;
    };
    return "WRONG MATERIAL";
}


Material::Material() 
    :   DataNode()
    ,   diffuseColor(0.8f, 0.8f, 0.8f, 1.0f)
    ,   specularColor(0.0f, 0.0f, 0.0f, 1.0f)
    ,   ambientColor(0.2f, 0.2f, 0.2f, 1.0f)
    ,   emissiveColor(0.0f, 0.0f, 0.0f, 1.0f)
    ,   shininess(1.0f)
    ,   isOpaque(false)
    ,   isTwoSided(false)
	,	isSetupLightmap(false)
	,	setupLightmapSize(1)
    ,   isFogEnabled(false)
    ,   fogDensity(0.006)
    ,   fogColor((float32)0x87 / 255.0f, (float32)0xbe / 255.0f, (float32)0xd7 / 255.0f, 1.0f)
{
//    if (scene)
//    {
//        DataNode * materialsNode = scene->GetMaterials();
//        materialsNode->AddNode(this);
//    }
    
    if (!uberShader)
    {
        uberShader = new UberShader();
        uberShader->LoadShader("~res:/Shaders/Default/materials.shader");
        
        
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE");
        uberShader->CompileShaderCombination("MATERIAL_DECAL");
        uberShader->CompileShaderCombination("MATERIAL_DETAIL");
        
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE;VERTEX_LIT");
        uberShader->CompileShaderCombination("MATERIAL_DECAL;VERTEX_LIT");
        uberShader->CompileShaderCombination("MATERIAL_DETAIL;VERTEX_LIT");
        
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;");
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;");
        uberShader->CompileShaderCombination("MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;GLOSS;");
    }
    
//    type = MATERIAL_UNLIT_TEXTURE;
//    shader = uberShader->GetShader("MATERIAL_TEXTURE");
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
        textures[tc] = 0;

    SetType(MATERIAL_UNLIT_TEXTURE);
}
    
void Material::SetScene(Scene * _scene)
{
    DVASSERT(scene == 0);
    scene = _scene;
}


int32 Material::Release()
{
    int32 retainCount = BaseObject::Release();
    return retainCount;
}

Material::~Material()
{
}
    
    
Material::eValidationResult Material::Validate(PolygonGroup * polygonGroup)
{
    RebuildShader();
    /*
        General check if number of attributes in shader is 
     */
    int32 format = polygonGroup->GetFormat();
    int32 formatBitCount = 0; // number of attributes in polygroup available in the shader
    for (int bit = 1; bit <= EVF_HIGHER_BIT; bit <<= 1)
    {
        if (format & bit)
        {
            if (shader->GetAttributeIndex((eVertexFormat)bit) != -1)
                formatBitCount++;
        }
    }
    // Check if we have more attributes 
    if (shader->GetAttributeCount() > formatBitCount)
    {
        //shader->Get
        
        
        
        return VALIDATE_INCOMPATIBLE;
    }
    
    
    return VALIDATE_COMPATIBLE;
}

void Material::RebuildShader()
{
    uniformTexture0 = -1;
    uniformTexture1 = -1;
    uniformLightPosition0 = -1;
    uniformMaterialLightAmbientColor = -1;
    uniformMaterialLightDiffuseColor = -1;
    uniformMaterialLightSpecularColor = -1;
    uniformMaterialSpecularShininess = -1;
    uniformLightIntensity0 = -1;
    uniformLightAttenuationQ = -1;
    uniformUvOffset = -1;
    uniformUvScale = -1;
    uniformFogDensity = -1;
    uniformFogColor = -1;
    
    String shaderCombileCombo = "MATERIAL_TEXTURE";
    
    switch (type) 
    {
        case MATERIAL_UNLIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE";
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
			shaderCombileCombo = "MATERIAL_LIGHTMAP";
			if(isSetupLightmap)
			{
				shaderCombileCombo = shaderCombileCombo + ";SETUP_LIGHTMAP";
			}
			break;
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            shaderCombileCombo = "MATERIAL_DECAL";
            break;
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            shaderCombileCombo = "MATERIAL_DETAIL";
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE;VERTEX_LIT";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR";
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            shaderCombileCombo = "MATERIAL_TEXTURE;PIXEL_LIT;DIFFUSE;SPECULAR;GLOSS";
            break;
        default:
            break;
    };
    if (isOpaque)
    {
        shaderCombileCombo = shaderCombileCombo + ";OPAQUE";
    }
    
    //if (isDistanceAttenuation)
    shaderCombileCombo = shaderCombileCombo + ";DISTANCE_ATTENUATION";
    
    if (isFogEnabled)
        shaderCombileCombo = shaderCombileCombo + ";VERTEX_FOG";

    // Get shader if combo unavailable compile it
    shader = uberShader->GetShader(shaderCombileCombo);
    
    switch (type) {
        case MATERIAL_UNLIT_TEXTURE:
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            uniformTexture0 = shader->FindUniformLocationByName("texture0");
            uniformTexture1 = shader->FindUniformLocationByName("texture1");
			uniformUvOffset = shader->FindUniformLocationByName("uvOffset");
			uniformUvScale = shader->FindUniformLocationByName("uvScale");
            
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            //
            uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
            uniformMaterialLightAmbientColor = shader->FindUniformLocationByName("materialLightAmbientColor");
            uniformMaterialLightDiffuseColor = shader->FindUniformLocationByName("materialLightDiffuseColor");
            uniformMaterialLightSpecularColor = shader->FindUniformLocationByName("materialLightSpecularColor");
            uniformMaterialSpecularShininess = shader->FindUniformLocationByName("materialSpecularShininess");
            uniformLightIntensity0 = shader->FindUniformLocationByName("lightIntensity0");
            uniformLightAttenuationQ = shader->FindUniformLocationByName("uniformLightAttenuationQ");
            
            break;
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE:
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR:
        case MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP:
            uniformTexture0 = shader->FindUniformLocationByName("texture0");
            uniformTexture1 = shader->FindUniformLocationByName("normalMapTexture");

            uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
            uniformMaterialLightAmbientColor = shader->FindUniformLocationByName("materialLightAmbientColor");
            uniformMaterialLightDiffuseColor = shader->FindUniformLocationByName("materialLightDiffuseColor");
            uniformMaterialLightSpecularColor = shader->FindUniformLocationByName("materialLightSpecularColor");
            uniformMaterialSpecularShininess = shader->FindUniformLocationByName("materialSpecularShininess");
            uniformLightIntensity0 = shader->FindUniformLocationByName("lightIntensity0");
            uniformLightAttenuationQ = shader->FindUniformLocationByName("uniformLightAttenuationQ");
            break;
        default:
            break;
    };
    
    if (isFogEnabled)
    {
        uniformFogDensity = shader->FindUniformLocationByName("fogDensity");
        uniformFogColor = shader->FindUniformLocationByName("fogColor");
    }
}
    
void Material::SetType(eType _type)
{
    type = _type;
    RebuildShader();
}
    
void Material::Save(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile)
{
    DataNode::Save(keyedArchive, sceneFile);
    
    keyedArchive->SetInt32("mat.texCount", TEXTURE_COUNT);
    for (int k = 0; k < TEXTURE_COUNT; ++k)
    {
        if (names[k].length() > 0)
        {
            String filename = sceneFile->AbsoluteToRelative(names[k]);
            keyedArchive->SetString(Format("mat.tex%d", k), filename);
            
            if(sceneFile->DebugLogEnabled())
                Logger::Debug("--- save material texture: %s", filename.c_str());
        }
    }
    
    keyedArchive->SetByteArrayAsType("mat.diffuse", diffuseColor);
    keyedArchive->SetByteArrayAsType("mat.ambient", ambientColor);
    keyedArchive->SetByteArrayAsType("mat.specular", specularColor);
    keyedArchive->SetByteArrayAsType("mat.emission", emissiveColor);
    keyedArchive->SetFloat("mat.shininess", shininess);
    
    keyedArchive->SetBool("mat.isOpaque", isOpaque);
    keyedArchive->SetBool("mat.isTwoSided", isTwoSided);
    
    keyedArchive->SetInt32("mat.type", type);
}
    
void Material::Load(KeyedArchive * keyedArchive, SceneFileV2 * sceneFile)
{
    Image::EnableAlphaPremultiplication(false);
    Texture::EnableMipmapGeneration();

    DataNode::Load(keyedArchive, sceneFile);

    int texCount = keyedArchive->GetInt32("mat.texCount");
    for (int k = 0; k < texCount; ++k)
    {
        String relativePathname = keyedArchive->GetString(Format("mat.tex%d", k));
        if (relativePathname.length() > 0)
        {
			String absolutePathname = relativePathname;
			if(!absolutePathname.empty() && absolutePathname[0] != '~') //not path like ~res:/Gfx...
			{
				absolutePathname = sceneFile->RelativeToAbsolute(relativePathname);
			}

            names[k] = absolutePathname;
            if(sceneFile->DebugLogEnabled())
                Logger::Debug("--- load material texture: %s abs:%s", relativePathname.c_str(), names[k].c_str());
            
            textures[k] = Texture::CreateFromFile(names[k]);
            if (textures[k])
            {
                textures[k]->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
            }
        }
        
//        if (names[k].size())
//        {
//            Logger::Debug("- texture: %s index:%d", names[k].c_str(), index);
//        } 
    }
    Texture::DisableMipmapGeneration();
    Image::EnableAlphaPremultiplication(true);
    
    
    diffuseColor = keyedArchive->GetByteArrayAsType("mat.diffuse", diffuseColor);
    ambientColor = keyedArchive->GetByteArrayAsType("mat.ambient", ambientColor);
    specularColor = keyedArchive->GetByteArrayAsType("mat.specular", specularColor);
    emissiveColor = keyedArchive->GetByteArrayAsType("mat.emission", emissiveColor);
    shininess = keyedArchive->GetFloat("mat.shininess", shininess);
    
    isOpaque = keyedArchive->GetBool("mat.isOpaque", isOpaque);
    isTwoSided = keyedArchive->GetBool("mat.isTwoSided", isTwoSided);

    eType mtype = (eType)keyedArchive->GetInt32("mat.type", type);
    SetType(mtype);
}

void Material::SetOpaque(bool _isOpaque)
{
    isOpaque = _isOpaque;
    RebuildShader();
}

bool Material::GetOpaque()
{
    return isOpaque;
}
void Material::SetTwoSided(bool _isTwoSided)
{
    isTwoSided = _isTwoSided;
}
    
bool Material::GetTwoSided()
{
    return isTwoSided;
}
    
void Material::SetAmbientColor(const Color & color)
{
    ambientColor = color;
}
void Material::SetDiffuseColor(const Color & color)
{
    diffuseColor = color;
}
void Material::SetSpecularColor(const Color & color)
{
    specularColor = color;
}
void Material::SetEmissiveColor(const Color & color)
{
    emissiveColor = color;
}

const Color & Material::GetAmbientColor() const
{
    return ambientColor;
}
const Color & Material::GetDiffuseColor() const
{
    return diffuseColor;
}
const Color & Material::GetSpecularColor() const
{
    return specularColor;
}
const Color & Material::GetEmissiveColor() const
{
    return emissiveColor;
}
    
void Material::SetShininess(float32 _shininess)
{
    shininess = _shininess;
}

float32 Material::GetShininess() const
{
    return shininess;
}
    
void Material::SetFog(bool _isFogEnabled)
{
    isFogEnabled = _isFogEnabled;
}
    
bool Material::IsFogEnabled() const
{
    return isFogEnabled;
}
    
void Material::SetFogDensity(float32 _fogDensity)
{
    fogDensity = _fogDensity;
}
    
float32 Material::GetFogDensity() const
{
    return fogDensity;
}

void Material::SetFogColor(const Color & _fogColor)
{
    fogColor = _fogColor;
}

const Color & Material::GetFogColor() const
{
    return fogColor;
}

void Material::Draw(PolygonGroup * group, InstanceMaterialState * instanceMaterialState)
{
	RenderManager::Instance()->SetRenderData(group->renderDataObject);

    RenderManager::Instance()->SetShader(shader);
        
    if (textures[Material::TEXTURE_DIFFUSE])
    {
        RenderManager::Instance()->SetTexture(textures[Material::TEXTURE_DIFFUSE], 0);
    }
        
    if (textures[Material::TEXTURE_DECAL]) // this is normal map as well
    {
        RenderManager::Instance()->SetTexture(textures[Material::TEXTURE_DECAL], 1);
    }
        
    if (isOpaque || isTwoSided)
    {
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE & (~RenderStateBlock::STATE_CULL));
    }else
    {
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
    }
    
    // render
    RenderManager::Instance()->FlushState();
    
    
    if (textures[Material::TEXTURE_DECAL])
    {
		if(uniformTexture0 != -1)
		{
			shader->SetUniformValue(uniformTexture0, 0);
		}
		if(uniformTexture1 != -1)
		{
			shader->SetUniformValue(uniformTexture1, 1);
		}
    }
    
	if(isSetupLightmap)
	{
		int32 lightmapSizePosition = shader->FindUniformLocationByName("lightmapSize");
		if (lightmapSizePosition != -1)
		{
			shader->SetUniformValue(lightmapSizePosition, (float32)setupLightmapSize); 
		}
	}

	if(MATERIAL_UNLIT_TEXTURE_LIGHTMAP == type)
	{
        if (uniformUvOffset != -1)
            shader->SetUniformValue(uniformUvOffset, uvOffset);
		if (uniformUvScale != -1)
            shader->SetUniformValue(uniformUvScale, uvScale);
	}
	

    if (instanceMaterialState)
    {
        Camera * camera = scene->GetCurrentCamera();
        LightNode * lightNode0 = instanceMaterialState->GetLight(0);
        if (lightNode0 && camera)
        {
            if (uniformLightPosition0 != -1)
            {
                const Matrix4 & matrix = camera->GetMatrix();
                Vector3 lightPosition0InCameraSpace = lightNode0->GetPosition() * matrix;
                
                shader->SetUniformValue(uniformLightPosition0, lightPosition0InCameraSpace); 
            }
            if (uniformMaterialLightAmbientColor != -1)
            {
                shader->SetUniformValue(uniformMaterialLightAmbientColor, lightNode0->GetAmbientColor() * GetAmbientColor());
            }
            if (uniformMaterialLightDiffuseColor != -1)
            {
                shader->SetUniformValue(uniformMaterialLightDiffuseColor, lightNode0->GetDiffuseColor() * GetDiffuseColor());
            }
            if (uniformMaterialLightSpecularColor != -1)
            {
                shader->SetUniformValue(uniformMaterialLightSpecularColor, lightNode0->GetSpecularColor() * GetSpecularColor());
            }
            if (uniformMaterialSpecularShininess != -1)
            {
                shader->SetUniformValue(uniformMaterialSpecularShininess, shininess);
            }
            
            if (uniformLightIntensity0 != -1)
            {
                shader->SetUniformValue(uniformLightIntensity0, lightNode0->GetIntensity());
            }
            if (uniformLightAttenuationQ != -1)
            {
                //shader->SetUniformValue(uniformLightAttenuationQ, lightNode0->GetAttenuation());
            }
        }
    }
    
    if (isFogEnabled)
    {
        if (uniformFogDensity != -1)
            shader->SetUniformValue(uniformFogDensity, fogDensity);
        if (uniformFogColor != -1)
            shader->SetUniformValue(uniformFogColor, fogColor);
    }
    
    // TODO: rethink this code
    if (group->renderDataObject->GetIndexBufferID() != 0)
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, 0);
	}else
	{
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, group->indexCount, EIF_16, group->indexArray);
	}
    
	RenderManager::Instance()->SetTexture(0, 1); 
	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);

}

void Material::SetSetupLightmap(bool _isSetupLightmap)
{
	if(isSetupLightmap != _isSetupLightmap)
	{
		isSetupLightmap = _isSetupLightmap;
		RebuildShader();
	}
}

bool Material::GetSetupLightmap()
{
	return isSetupLightmap;
}

void Material::SetSetupLightmapSize(int32 _setupLightmapSize)
{
	setupLightmapSize = _setupLightmapSize;
}
    
void Material::SetTexture(eTextureLevel level, const String & textureName)
{
    SafeRelease(textures[level]);
    names[level] = "";
    
    Texture *t = Texture::CreateFromFile(textureName);
    if(t)
    {
		t->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
        textures[level] = t;
        names[level] = textureName;
    }
}


};
