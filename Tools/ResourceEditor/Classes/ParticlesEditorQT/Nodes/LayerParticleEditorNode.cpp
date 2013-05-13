//
//  LayerParticleEditorNode.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 12/4/12.
//
//

#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLayer.h"

#include "LayerParticleEditorNode.h"
#include "ForceParticleEditorNode.h"
#include "InnerEmitterParticleEditorNode.h"

using namespace DAVA;

LayerParticleEditorNode::LayerParticleEditorNode(EmitterParticleEditorNode* emitterEditorNode,
                                                 ParticleLayer* layer) :
    EmitterContainerNode(emitterEditorNode->GetRootNode(), emitterEditorNode->GetEmitterNode(),
                         "Emitter Layer")
{
    this->emitterEditorNode = emitterEditorNode;
    this->layer = layer;
}

QString LayerParticleEditorNode::GetName() const
{
	if (this->layer)
	{
		return QString::fromStdString(this->layer->layerName);
	}
	
	return QString();
}

int32 LayerParticleEditorNode::GetLayerIndex() const
{
    ParticleEmitter* emitter = GetParticleEmitter();
    if (!emitter)
    {
        return -1;
    }
    
    const Vector<ParticleLayer*>& layers = emitter->GetLayers();
    int32 layersCount = layers.size();
    
    for (int32 i = 0; i < layersCount; i ++)
    {
        if (layers[i] == this->layer)
        {
            return i;
        }
    }
    
    return -1;
}

int32 LayerParticleEditorNode::GetForcesCount() const
{
    int32 forcesCount = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        ForceParticleEditorNode* childNode = dynamic_cast<ForceParticleEditorNode*>(*iter);
        if (childNode)
        {
            forcesCount ++;
        }
    }
    
    return forcesCount;
}

void LayerParticleEditorNode::UpdateForcesIndices()
{
    // Re-synchronize new Forces List with the appropriate Tree Nodes.
    ParticleLayer* curLayer = this->GetLayer();
    
    int32 processedChildNodes = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        ForceParticleEditorNode* childNode = dynamic_cast<ForceParticleEditorNode*>(*iter);
        if (!childNode)
        {
            continue;
        }
        
        childNode->UpdateForceIndex(processedChildNodes);
        childNode->SetNodeName(QString("Force %1").arg(processedChildNodes + 1));
        processedChildNodes ++;
    }

    // We have to update exactly the same child nodes as the forces count we have.
    int32 forcesCount = curLayer->forces.size();
    DVASSERT(forcesCount == processedChildNodes);
}

int32 LayerParticleEditorNode::GetInnerEmittersCount()
{
	int32 innerEmittersCount = 0;
    for (List<BaseParticleEditorNode*>::const_iterator iter = GetChildren().begin(); iter != GetChildren().end();
         iter ++)
    {
        InnerEmitterParticleEditorNode* childNode = dynamic_cast<InnerEmitterParticleEditorNode*>(*iter);
        if (childNode)
        {
            innerEmittersCount ++;
        }
    }
    
    return innerEmittersCount;
}