//
//  LayerParticleEditorNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/4/12.
//
//

#ifndef __ResourceEditorQt__LayerParticleEditorNode__
#define __ResourceEditorQt__LayerParticleEditorNode__

#include "EmitterContainerNode.h"
#include "EmitterParticleEditorNode.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA {
    
// Node which represents particle Layer node.
class LayerParticleEditorNode : public EmitterContainerNode
{
public:
    LayerParticleEditorNode(EmitterParticleEditorNode* emitterEditorNode,
                            ParticleLayer* layer);

    EmitterParticleEditorNode* GetEmitterEditorNode() const {return emitterEditorNode;};
    ParticleLayer* GetLayer() const {return layer;};

	virtual QString GetName() const;

    // Get the layer index in the Particle Emitter.
    int32 GetLayerIndex() const;

    // Get the count of forces added to the layer.
    int32 GetForcesCount() const;

	// Get the count of Inner Emitters added to the layer.
	int32 GetInnerEmittersCount();

    // Update the forces indices.
    void UpdateForcesIndices();
	
	// Update the Emitter Editor node.
	void UpdateEmitterEditorNode(EmitterParticleEditorNode* newNode)
	{
		this->emitterNode = newNode->GetEmitterNode();
		this->emitterEditorNode = newNode;
	};

protected:
    EmitterParticleEditorNode* emitterEditorNode;
    ParticleLayer* layer;
};

};

#endif /* defined(__ResourceEditorQt__LayerParticleEditorNode__) */
