//
//  ParticleEditorEmitterContainerNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/4/12.
//
//

#ifndef __ResourceEditorQt__ParticleEditorEmitterContainerNode__
#define __ResourceEditorQt__ParticleEditorEmitterContainerNode__

#include "BaseParticleEditorNode.h"
#include "Scene3D/ParticleEmitterNode.h"

namespace DAVA {
    
// Base node for all emitter-contained Particle Editor nodes.
class EmitterContainerNode : public BaseParticleEditorNode
{
public:
    EmitterContainerNode(ParticleEffectNode* rootNode, ParticleEmitterNode* emitter,
                                  const QString& nodeName);
    virtual ~EmitterContainerNode();
        
    ParticleEmitterNode* GetEmitterNode() const {return emitter;};
    void UpdateEmitter(ParticleEmitterNode* emitterNode) {this->emitter = emitterNode;};

protected:
    ParticleEmitterNode* emitter;
};
    
};


#endif /* defined(__ResourceEditorQt__ParticleEditorEmitterContainerNode__) */
