//
//  EmitterParticleEditorNode.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__EmitterParticleEditorNode__
#define __ResourceEditorQt__EmitterParticleEditorNode__

#include "EmitterContainerNode.h"

namespace DAVA {
    
// Emitter Particle Editor node.
class EmitterParticleEditorNode : public EmitterContainerNode
{
public:
    EmitterParticleEditorNode(Entity* rootNode, Entity* emitter,
                              const QString& nodeName);
    
    // Get the layers count.
    int32 GetLayersCount() const;
};

};

#endif /* defined(__ResourceEditorQt__EmitterParticleEditorNode__) */
