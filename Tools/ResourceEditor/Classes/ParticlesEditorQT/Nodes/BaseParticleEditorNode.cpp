//
//  BaseParticleEditorNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "BaseParticleEditorNode.h"
using namespace DAVA;

BaseParticleEditorNode::BaseParticleEditorNode(ParticleEffectNode* rootNode) :
    ExtraUserData()
{
    this->isMarkedForSelection = false;
    this->rootNode = rootNode;
}

BaseParticleEditorNode::~BaseParticleEditorNode()
{
    Cleanup();
}

void BaseParticleEditorNode::Cleanup()
{
    for (PARTICLEEDITORNODESLISTITER iter = childNodes.begin(); iter != childNodes.end();
         iter ++)
    {
        SAFE_DELETE(*iter);
    }
    
    childNodes.clear();
}

void BaseParticleEditorNode::AddChildNode(BaseParticleEditorNode* childNode)
{
    if (!childNode)
    {
        return;
    }
    
    this->childNodes.push_back(childNode);
}

void BaseParticleEditorNode::RemoveChildNode(BaseParticleEditorNode* childNode)
{
    this->childNodes.remove(childNode);
    SAFE_DELETE(childNode);
}
