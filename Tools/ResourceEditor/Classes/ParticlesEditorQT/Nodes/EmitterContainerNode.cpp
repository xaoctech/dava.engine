//
//  EmitterContainerNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/4/12.
//
//

#include "EmitterContainerNode.h"

#include "Scene3D/Components/ComponentHelpers.h"

using namespace DAVA;

EmitterContainerNode::EmitterContainerNode(Entity* rootNode, Entity* emitterNode, const QString& nodeName) :
    BaseParticleEditorNode(rootNode)
{
    this->nodeName = nodeName;
	this->emitterNode = NULL;
	ParticleEmitter * emitter = GetEmitter(emitterNode);
    DVASSERT(emitter);
	this->emitterNode = emitterNode;
}

EmitterContainerNode::~EmitterContainerNode()
{
    this->emitterNode = NULL;
}

ParticleEmitter * EmitterContainerNode::GetParticleEmitter() const 
{
	return GetEmitter(emitterNode);
}