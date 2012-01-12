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

namespace DAVA 
{
    
REGISTER_CLASS(LightNode);

LightNode::LightNode(Scene * _scene)
    :   SceneNode(_scene)
    ,   type(ET_DIRECTIONAL)
    ,   color(1.0f, 1.0f, 1.0f, 1.0f)
{
	r = 1.0f;
}
    
LightNode::~LightNode()
{
    
}
    
void LightNode::Draw()
{	
	if (debugFlags > 0)
	{
		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
		Matrix4 finalMatrix = worldTransform * prevMatrix;
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
		
		RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_CULL); 
		RenderManager::Instance()->FlushState();
		
		
		if (debugFlags & DEBUG_DRAW_AABOX_CORNERS)
		{
			RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			RenderHelper::Instance()->DrawCornerBox(AABBox3(Vector3(-r,-r,-r),Vector3(r,r,r)));
		}
		if (debugFlags & DEBUG_DRAW_LIGHT_NODE)
		{
			RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
			RenderHelper::Instance()->DrawSphere(r);
		}

		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);	
		
	}
	SceneNode::Draw();
}
    
void LightNode::SetType(DAVA::LightNode::eType _type)
{
    type = _type;
}

void LightNode::SetColor(DAVA::Color _color)
{
    color = _color;
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
    lightNode->color = color;
    
    return dstNode;
}

LightNode::eType LightNode::GetType() const
{
    return type;
}
    
const Color & LightNode::GetColor() const
{
    return color;
}
    
float32 LightNode::GetRadius(void)
{
	return r;
}
	
};
