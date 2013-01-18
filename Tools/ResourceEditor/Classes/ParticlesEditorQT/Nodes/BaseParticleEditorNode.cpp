//
//  BaseParticleEditorNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "BaseParticleEditorNode.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
using namespace DAVA;

BaseParticleEditorNode::BaseParticleEditorNode(SceneNode* rootNode) :
    ExtraUserData()
{
	Component *effectComponent = rootNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT);
	DVASSERT(effectComponent);

    this->isMarkedForSelection = false;
    this->rootNode = rootNode;
}

BaseParticleEditorNode::~BaseParticleEditorNode()
{
    Cleanup();
}

void BaseParticleEditorNode::Cleanup()
{
    for (List<BaseParticleEditorNode*>::iterator iter = childNodes.begin(); iter != childNodes.end();
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

ParticleEffectComponent* BaseParticleEditorNode::GetParticleEffectComponent() const
{
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(rootNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
	return effectComponent;
}
