//
//  InnerEmitterParticleEditorNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 4/30/13.
//
//

#ifndef __INNER_EMITTER_PARTICLE_EDITOR_NODE_H__
#define __INNER_EMITTER_PARTICLE_EDITOR_NODE_H__

#include "EmitterContainerNode.h"
#include "LayerParticleEditorNode.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA {

// Node which represents Inner Emitter node (inside the Layer one).
class InnerEmitterParticleEditorNode : public EmitterContainerNode
{
public:
    InnerEmitterParticleEditorNode(LayerParticleEditorNode* layerEditorNode);
	
	// Access to Inner Emitter.
	ParticleEmitter* GetInnerEmitter();

protected:
    LayerParticleEditorNode* layerEditorNode;
};

};

#endif /* defined(__INNER_EMITTER_PARTICLE_EDITOR_NODE_H__) */
