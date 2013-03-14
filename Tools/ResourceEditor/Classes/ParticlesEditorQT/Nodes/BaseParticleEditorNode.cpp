//
//  BaseParticleEditorNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "BaseParticleEditorNode.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "FileSystem/KeyedArchive.h"
using namespace DAVA;

BaseParticleEditorNode::BaseParticleEditorNode(Entity* rootNode) :
    ExtraUserData()
{
	Component *effectComponent = rootNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT);
	DVASSERT(effectComponent);

    this->isMarkedForSelection = false;
    this->rootNode = rootNode;
    SetParentNode(NULL);

	extraData = new KeyedArchive();
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
	SetParentNode(NULL);

	ClearExtraData();
	SafeRelease(extraData);
}

void BaseParticleEditorNode::AddChildNode(BaseParticleEditorNode* childNode)
{
    if (!childNode)
    {
        return;
    }
    
	childNode->SetParentNode(this);
    this->childNodes.push_back(childNode);
}

ParticleEffectComponent* BaseParticleEditorNode::GetParticleEffectComponent() const
{
	ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(rootNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	DVASSERT(effectComponent);
	return effectComponent;
}

void BaseParticleEditorNode::AddChildNodeAbove(BaseParticleEditorNode* childNode, BaseParticleEditorNode* childNodeToMoveAbove)
{
	AddChildNode(childNode);
	if (childNodeToMoveAbove)
	{
		MoveChildNode(childNode, childNodeToMoveAbove);
	}
}

void BaseParticleEditorNode::RemoveChildNode(BaseParticleEditorNode* childNode, bool needDeleteNode)
{
    this->childNodes.remove(childNode);
	
	if (needDeleteNode)
	{
		childNode->SetParentNode(NULL);
		SAFE_DELETE(childNode);
	}
}

void BaseParticleEditorNode::MoveChildNode(BaseParticleEditorNode* childNode, BaseParticleEditorNode* childNodeToMoveAbove)
{
	PARTICLEEDITORNODESLIST::iterator curPositionIter = std::find(this->childNodes.begin(),
																  this->childNodes.end(),
																  childNode);
	PARTICLEEDITORNODESLIST::iterator newPositionIter = std::find(this->childNodes.begin(),
																  this->childNodes.end(),
																  childNodeToMoveAbove);

	if (curPositionIter == this->childNodes.end() ||
		newPositionIter == this->childNodes.end() ||
		curPositionIter == newPositionIter)
	{
		// No way to move.
		return;
	}

	childNodes.remove(childNode);
	
	// Re-calculate the new position iter - it might be changed during remove.
	newPositionIter = std::find(this->childNodes.begin(), this->childNodes.end(), childNodeToMoveAbove);
	childNodes.insert(newPositionIter, childNode);
}

KeyedArchive* BaseParticleEditorNode::GetExtraData()
{
	return extraData;
}

void BaseParticleEditorNode::ClearExtraData()
{
	extraData->DeleteAllKeys();
}
