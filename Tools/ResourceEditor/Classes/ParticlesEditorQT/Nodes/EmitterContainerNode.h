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
    EmitterContainerNode(Entity* rootNode, Entity* emitter,
                                  const QString& nodeName);
    virtual ~EmitterContainerNode();
        
    Entity* GetEmitterNode() const {return emitterNode;};

	ParticleEmitter * GetParticleEmitter() const; 

protected:
    Entity* emitterNode;
};
    
};


#endif /* defined(__ResourceEditorQt__ParticleEditorEmitterContainerNode__) */
