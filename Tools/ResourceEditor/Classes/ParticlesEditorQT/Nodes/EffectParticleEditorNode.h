//
//  EffectParticleEditorNode.h.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/27/12.
//
//

#ifndef __ResourceEditorQt__EffectParticleEditorNode__h__
#define __ResourceEditorQt__EffectParticleEditorNode__h__

#include "BaseParticleEditorNode.h"

namespace DAVA {

// Root Particle Editor node.
class EffectParticleEditorNode : public BaseParticleEditorNode
{
public:
    EffectParticleEditorNode(Entity* rootNode);
    virtual ~EffectParticleEditorNode();
    
    // Get the count of emitters in the root node.
    int32 GetEmittersCount() const;
};

};

#endif /* defined(__ResourceEditorQt__EffectParticleEditorNode__h__) */
