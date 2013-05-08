//
//  InnerEmitterParticleEditorNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 4/30/13.
//
//

#include "InnerEmitterParticleEditorNode.h"

namespace DAVA {

InnerEmitterParticleEditorNode::InnerEmitterParticleEditorNode(LayerParticleEditorNode* layerEditorNode) :
	EmitterContainerNode(layerEditorNode->GetRootNode(),
                         layerEditorNode->GetEmitterEditorNode()->GetEmitterNode(),
                         "Inner Emitter")
{
	this->layerEditorNode = layerEditorNode;
}

ParticleEmitter* InnerEmitterParticleEditorNode::GetInnerEmitter()
{
	if (layerEditorNode && layerEditorNode->GetLayer())
	{
		return layerEditorNode->GetLayer()->GetInnerEmitter();
	}
	
	return NULL;
}
};