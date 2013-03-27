//
//  EffectParticleEditorNode.h.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/27/12.
//
//

#include "EffectParticleEditorNode.h"
#include "EmitterParticleEditorNode.h"
using namespace DAVA;

EffectParticleEditorNode::EffectParticleEditorNode(Entity* rootNode) :
    BaseParticleEditorNode(rootNode)
{
}

EffectParticleEditorNode::~EffectParticleEditorNode()
{
}

int32 EffectParticleEditorNode::GetEmittersCount() const
{
    int32 emittersCount = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        EmitterParticleEditorNode* childNode = dynamic_cast<EmitterParticleEditorNode*>(*iter);
        if (childNode)
        {
            emittersCount ++;
        }
    }
    
    return emittersCount;
}