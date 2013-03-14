//
//  EmitterParticleEditorNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "EmitterParticleEditorNode.h"
#include "LayerParticleEditorNode.h"

using namespace DAVA;

EmitterParticleEditorNode::EmitterParticleEditorNode(Entity* rootNode, Entity* emitter,
                                                     const QString& nodeName) :
    EmitterContainerNode(rootNode, emitter, nodeName)
{
}

int32 EmitterParticleEditorNode::GetLayersCount() const
{
    int32 layersCount = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        LayerParticleEditorNode* childNode = dynamic_cast<LayerParticleEditorNode*>(*iter);
        if (childNode)
        {
            layersCount ++;
        }
    }
    
    return layersCount;
    
}
