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

namespace DAVA {

class ParticleEmitterComponent;

// Base node for all emitter-contained Particle Editor nodes.
class EmitterContainerNode : public BaseParticleEditorNode
{
public:
    EmitterContainerNode(ParticleEffectNode* rootNode, SceneNode* emitter,
                                  const QString& nodeName);
    virtual ~EmitterContainerNode();
        
    SceneNode* GetEmitterNode() const {return emitter;};

    ParticleEmitterComponent * GetParticleComponent() const; 

protected:
    SceneNode* emitter;
};
    
};


#endif /* defined(__ResourceEditorQt__ParticleEditorEmitterContainerNode__) */
