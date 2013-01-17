//
//  EmitterContainerNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/4/12.
//
//

#include "EmitterContainerNode.h"

#include "Scene3D/Components/ParticleEmitterComponent.h"

using namespace DAVA;

EmitterContainerNode::EmitterContainerNode(ParticleEffectNode* rootNode, SceneNode* emitter, const QString& nodeName) :
    BaseParticleEditorNode(rootNode)
{
    this->nodeName = nodeName;
	this->emitter = NULL;
	ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(emitter->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    DVASSERT(emitterComponent);
	this->emitter = emitter;
	
}

EmitterContainerNode::~EmitterContainerNode()
{
    this->emitter = NULL;
}

ParticleEmitterComponent * EmitterContainerNode::GetParticleComponent() const 
{
	return static_cast<ParticleEmitterComponent*>(emitter->components[Component::PARTICLE_EMITTER_COMPONENT]);
}