//
//  ForceParticleEmitterNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/5/12.
//
//

#ifndef __ResourceEditorQt__ForceParticleEmitterNode__
#define __ResourceEditorQt__ForceParticleEmitterNode__

#include "BaseParticleEditorNode.h"
#include "LayerParticleEditorNode.h"

namespace DAVA {
    
// Node which represents particle force node.
class ForceParticleEditorNode : public EmitterContainerNode
{
public:
    ForceParticleEditorNode(LayerParticleEditorNode* layerEditorNode, int32 forceIndex);

    LayerParticleEditorNode* GetLayerEditorNode() const {return this->layerEditorNode;};
    ParticleLayer* GetLayer() const;

    // Get/set the Force Index.
    int32 GetForceIndex() const {return this->forceIndex;};
    void UpdateForceIndex(int32 newForceIndex) {this->forceIndex = newForceIndex;};

protected:
    // All the data sent in the constructor needs to be stored.
    LayerParticleEditorNode* layerEditorNode;
    int32 forceIndex;
};

};

#endif /* defined(__ResourceEditorQt__ForceParticleEmitterNode__) */
