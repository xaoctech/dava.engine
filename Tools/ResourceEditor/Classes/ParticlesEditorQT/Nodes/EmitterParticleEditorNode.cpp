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

EmitterParticleEditorNode::EmitterParticleEditorNode(ParticleEffectNode* rootNode, ParticleEmitterNode* emitter,
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

void EmitterParticleEditorNode::UpdateLayerNames()
{
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        LayerParticleEditorNode* childNode = dynamic_cast<LayerParticleEditorNode*>(*iter);
        if (!childNode)
        {
            continue;
        }
        
        int layerIndex = childNode->GetLayerIndex();
        (*iter)->SetNodeName(QString("Layer %1").arg(layerIndex + 1));
    }
}
