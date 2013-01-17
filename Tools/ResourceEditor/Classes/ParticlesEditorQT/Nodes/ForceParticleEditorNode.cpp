//
//  ForceParticleEmitterNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/5/12.
//
//

#include "ForceParticleEditorNode.h"

using namespace DAVA;

ForceParticleEditorNode::ForceParticleEditorNode(LayerParticleEditorNode* layerEditorNode, int32 forceIndex) :
    EmitterContainerNode(layerEditorNode->GetRootNode(),
                         layerEditorNode->GetEmitterEditorNode()->GetEmitterNode(),
                         "Force")
{
    this->layerEditorNode = layerEditorNode;
}

ParticleLayer* ForceParticleEditorNode::GetLayer() const
{
    if (this->layerEditorNode)
    {
        return this->layerEditorNode->GetLayer();
    }
    
    return NULL;
}