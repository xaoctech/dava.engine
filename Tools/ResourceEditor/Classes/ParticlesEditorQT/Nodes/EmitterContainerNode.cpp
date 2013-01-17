//
//  EmitterContainerNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/4/12.
//
//

#include "EmitterContainerNode.h"

using namespace DAVA;

EmitterContainerNode::EmitterContainerNode(ParticleEffectNode* rootNode, ParticleEmitterNode* emitter, const QString& nodeName) :
    BaseParticleEditorNode(rootNode)
{
    this->nodeName = nodeName;
    this->emitter = emitter;
}

EmitterContainerNode::~EmitterContainerNode()
{
    this->emitter = NULL;
}