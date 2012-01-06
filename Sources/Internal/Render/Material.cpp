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
    }
    
    type = MATERIAL_UNLIT;
    shader = uberShader->GetShader("MATERIAL_TEXTURE");
    for (int32 tc = 0; tc < TEXTURE_COUNT; ++tc)
        textures[tc] = 0;
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
    type = _type;
    String shaderCombileCombo = "MATERIAL_TEXTURE";
    switch (type) {
        case MATERIAL_UNLIT:
            shaderCombileCombo = "MATERIAL_TEXTURE";
            break;
        case MATERIAL_UNLIT_DECAL:
            shaderCombileCombo = "MATERIAL_DECAL";
            break;
        case MATERIAL_UNLIT_DETAIL:
            shaderCombileCombo = "MATERIAL_DETAIL";
            break;
        default:
            break;
    };
    shader = uberShader->GetShader(shaderCombileCombo);
}
    
void Material::Save(KeyedArchive * keyedArchive)
{
    DataNode::Save(keyedArchive);
    
    keyedArchive->SetInt32("mat.texCount", TEXTURE_COUNT);
    for (int k = 0; k < TEXTURE_COUNT; ++k)
    {
        keyedArchive->SetString(Format("mat.tex%d", k), names[k].c_str());
    }
    
    keyedArchive->SetByteArrayAsType("diffuse", diffuse);
    keyedArchive->SetByteArrayAsType("ambient", ambient);
    keyedArchive->SetByteArrayAsType("specular", specular);
    keyedArchive->SetByteArrayAsType("emission", emission);
    keyedArchive->SetBool("isOpaque", isOpaque);
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
        textures[k] = Texture::CreateFromFile(names[k]);
        //}
//        if (names[k].size())
//        {
//            Logger::Debug("- texture: %s index:%d", names[k].c_str(), index);
//        } 
    }
    
    keyedArchive->GetByteArrayAsType("diffuse", diffuse);
    keyedArchive->GetByteArrayAsType("ambient", ambient);
    keyedArchive->GetByteArrayAsType("specular", specular);
    keyedArchive->GetByteArrayAsType("emission", emission);
    
    isOpaque = keyedArchive->GetBool("isOpaque", isOpaque);
}



};