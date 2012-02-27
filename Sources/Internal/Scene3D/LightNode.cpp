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

#include "Scene3D/LightNode.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Scene.h"

namespace DAVA 
{
    
REGISTER_CLASS(LightNode);

LightNode::LightNode(Scene * _scene)
:	SceneNode(_scene),
	type(TYPE_DIRECTIONAL),
	diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
    specularColor(1.0f, 1.0f, 1.0f, 1.0f)
{
}
    
LightNode::~LightNode()
{
    
}
    
void LightNode::SetType(DAVA::LightNode::eType _type)
{
    type = _type;
}

void LightNode::SetDiffuseColor(const Color & _color)
{
    diffuseColor = _color;
}

void LightNode::SetSpecularColor(const Color & _color)
{
    specularColor = _color;
}

SceneNode* LightNode::Clone(SceneNode *dstNode)
{
    if(!dstNode)
    {
        dstNode = new LightNode(GetScene());
    }
    
    SceneNode::Clone(dstNode);
    
    LightNode *lightNode = (LightNode *)dstNode;
    lightNode->type = type;
    lightNode->diffuseColor = diffuseColor;
    lightNode->specularColor = specularColor;
    
    return dstNode;
}
    
void LightNode::Update(float32 timeElapsed)
{
    bool needUpdateVars = false;
    if (!(flags & NODE_WORLD_MATRIX_ACTUAL)) 
    {
        needUpdateVars = true;
        GetScene()->AddFlag(SceneNode::SCENE_LIGHTS_MODIFIED);
    }
    SceneNode::Update(timeElapsed);
    
    if (needUpdateVars)
    {
        position = Vector3(0.0f, 0.0f, 0.0f) * GetWorldTransform();
        Matrix3 rotationPart(GetWorldTransform());
        direction = Vector3(0.0, -1.0f, 0.0f) * rotationPart;
        direction.Normalize();
    }
}

LightNode::eType LightNode::GetType() const
{
    return type;
}
const Vector3 & LightNode::GetPosition() const
{
    return position; 
}

const Vector3 & LightNode::GetDirection() const
{
    return direction;
}
    
const Color & LightNode::GetDiffuseColor() const
{
    return diffuseColor;
}
    
const Color & LightNode::GetSpecularColor() const
{
    return specularColor;
}

void LightNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
	SceneNode::Save(archive, sceneFile);
	
	archive->SetInt32("type", type);
	archive->SetFloat("color.r", diffuseColor.r);
	archive->SetFloat("color.g", diffuseColor.g);
	archive->SetFloat("color.b", diffuseColor.b);
	archive->SetFloat("color.a", diffuseColor.a);

	archive->SetFloat("specColor.r", specularColor.r);
	archive->SetFloat("specColor.g", specularColor.g);
	archive->SetFloat("specColor.b", specularColor.b);
	archive->SetFloat("specColor.a", specularColor.a);
}

void LightNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFile)
{
    SceneNode::Load(archive, sceneFile);

    type = (eType)archive->GetInt32("type");
    diffuseColor.r = archive->GetFloat("color.r", diffuseColor.r);
    diffuseColor.g = archive->GetFloat("color.g", diffuseColor.g);
    diffuseColor.b = archive->GetFloat("color.b", diffuseColor.b);
    diffuseColor.a = archive->GetFloat("color.a", diffuseColor.a);
    
    specularColor.r = archive->GetFloat("specColor.r", specularColor.r);
    specularColor.g = archive->GetFloat("specColor.g", specularColor.g);
    specularColor.b = archive->GetFloat("specColor.b", specularColor.b);
    specularColor.a = archive->GetFloat("specColor.a", specularColor.a);
}

void LightNode::Draw()
{
    SceneNode::Draw();
    
    if (debugFlags != DEBUG_DRAW_NONE)
    {
        if (!(flags & SceneNode::NODE_VISIBLE))return;

        RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
        RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE); 
        
        RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f); 
        RenderHelper::Instance()->DrawLine(position, position + direction * 20);        
        
        RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
        RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

};
