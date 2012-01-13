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
#include "Scene3D/DataNode.h"
#include "Scene3D/Scene.h"
#include "Render/Shader.h"

namespace DAVA 
{
    
REGISTER_CLASS(Material);
    
UberShader * Material::uberShader = 0;

Material::Material(Scene * _scene) 
    :   DataNode(_scene)
    ,   diffuse(1.0f, 1.0f, 1.0f, 1.0f)
    ,   specular(1.0f, 1.0f, 1.0f, 1.0f)
    ,   ambient(1.0f, 1.0f, 1.0f, 1.0f)
    ,   emission(1.0f, 1.0f, 1.0f, 1.0f)
    ,   isOpaque(false)
{
    if (scene)
    {
        DataNode * materialsNode = scene->GetMaterials();
        materialsNode->AddNode(this);
    }
    
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
    if (scene)
    {
        DataNode * materialsNode = scene->GetMaterials();
        materialsNode->AddNode(this);
    }
}


int32 Material::Release()
{
    int32 retainCount = BaseObject::Release();
    if (retainCount == 1)
    {
        DataNode * materialsNode = scene->GetMaterials();
        materialsNode->RemoveNode(this);
    }
    return retainCount;
}

Material::~Material()
{
}
    
void Material::SetType(eType _type)
{
    uniformTexture0 = -1;
    uniformTexture1 = -1;
    uniformLightPosition0 = -1;
    
    type = _type;
    String shaderCombileCombo = "MATERIAL_TEXTURE";
    switch (type) {
        case MATERIAL_UNLIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE";
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
            shaderCombileCombo = "MATERIAL_DECAL";
            break;
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            shaderCombileCombo = "MATERIAL_DETAIL";
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            shaderCombileCombo = "MATERIAL_TEXTURE;VERTEX_LIT";
            break;
            
        default:
            break;
    };
    shader = uberShader->GetShader(shaderCombileCombo);

    switch (type) {
        case MATERIAL_UNLIT_TEXTURE:
            break;
        case MATERIAL_UNLIT_TEXTURE_LIGHTMAP:
        case MATERIAL_UNLIT_TEXTURE_DECAL:
        case MATERIAL_UNLIT_TEXTURE_DETAIL:
            //shader->SetT
            uniformTexture0 = shader->FindUniformLocationByName("texture0");
            uniformTexture1 = shader->FindUniformLocationByName("texture1");
            
            break;
        case MATERIAL_VERTEX_LIT_TEXTURE:
            //
            shaderCombileCombo = "MATERIAL_TEXTURE;VERTEX_LIT";
            uniformLightPosition0 = shader->FindUniformLocationByName("lightPosition0");
    
            break;
        default:
            break;
    };

}
    
void Material::Save(KeyedArchive * keyedArchive)
{
    DataNode::Save(keyedArchive);
    
    keyedArchive->SetInt32("mat.texCount", TEXTURE_COUNT);
    for (int k = 0; k < TEXTURE_COUNT; ++k)
    {
        keyedArchive->SetString(Format("mat.tex%d", k), names[k].c_str());
    }
    
    keyedArchive->SetByteArrayAsType("mat.diffuse", diffuse);
    keyedArchive->SetByteArrayAsType("mat.ambient", ambient);
    keyedArchive->SetByteArrayAsType("mat.specular", specular);
    keyedArchive->SetByteArrayAsType("mat.emission", emission);
    keyedArchive->SetBool("mat.isOpaque", isOpaque);
    
    
    keyedArchive->SetInt32("mat.type", type);
}
    
void Material::Load(KeyedArchive * keyedArchive)
{
    DataNode::Load(keyedArchive);

    int texCount = keyedArchive->GetInt32("mat.texCount");
    for (int k = 0; k < texCount; ++k)
    {
        names[k] = keyedArchive->GetString(Format("mat.tex%d", k));
        //if (textures[k].length())
        //{
        Texture::EnableMipmapGeneration();
        textures[k] = Texture::CreateFromFile(pathBase + names[k]);
        if (textures[k])
        {
            textures[k]->SetWrapMode(Texture::WRAP_REPEAT, Texture::WRAP_REPEAT);
        }
        Texture::DisableMipmapGeneration();
        
//        if (names[k].size())
//        {
//            Logger::Debug("- texture: %s index:%d", names[k].c_str(), index);
//        } 
    }
    
    keyedArchive->GetByteArrayAsType("mat.diffuse", diffuse);
    keyedArchive->GetByteArrayAsType("mat.ambient", ambient);
    keyedArchive->GetByteArrayAsType("mat.specular", specular);
    keyedArchive->GetByteArrayAsType("mat.emission", emission);
    
    isOpaque = keyedArchive->GetBool("mat.isOpaque", isOpaque);

    eType mtype = (eType)keyedArchive->GetInt32("mat.type", type);
    SetType(mtype);
}



};