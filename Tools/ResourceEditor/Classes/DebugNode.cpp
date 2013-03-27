/*
 *  DebugNode.cpp
 *  WoTSniperiPhone
 *
 *  Created by Yury Danilov on 29/11/11.
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "DebugNode.h"


DebugNode::DebugNode(std::vector<float32> &data)
    : Entity()
{
	verts = data;
	
	for (int32 i = 0; i < (int32)verts.size() / 3; i++)
	{
		for (int j = 0; j < 4; j++)
			colors.push_back(1.0f);
	}
	renderData = NULL;
	renderData = new RenderDataObject();
	renderData->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, 0, verts.data());
	renderData->SetStream(EVF_COLOR, TYPE_FLOAT, 4, 0, colors.data());
	isDraw = true;
}

DebugNode::~DebugNode()
{
	SafeRelease(renderData); 
}

void DebugNode::Draw()
{
	Entity::Draw();
	if (!isDraw)
		return;
	if (verts.size() == 0)
		return;
	
	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 meshFinalMatrix = GetWorldTransform() * prevMatrix;
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);	
    RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);

    RenderManager::Instance()->SetRenderData(renderData);
	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_LINELIST, 0, verts.size()/3);
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);    
}

