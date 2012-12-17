//
//  ParticlesEditorController.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "ParticlesEditorController.h"

using namespace DAVA;

ParticlesEditorController::ParticlesEditorController(QObject* parent) :
    QObject(parent)
{
    this->selectedNode = NULL;
}

ParticlesEditorController::~ParticlesEditorController()
{
    Cleanup();
}

EffectParticleEditorNode* ParticlesEditorController::RegisterParticleEffectNode(ParticleEffectNode* effectNode, bool autoStart)
{
    if (!effectNode)
    {
        Logger::Warning("ParticlesEditorController::RegisterParticleEffectNode(): node is NULL!");
        return NULL;
    }

    EffectParticleEditorNode* rootNode = new EffectParticleEditorNode(effectNode);
    this->particleEffectNodes[effectNode] = rootNode;
    if (autoStart)
    {
        effectNode->Start();
    }

    return rootNode;
}

void ParticlesEditorController::UnregiserParticleEffectNode(ParticleEffectNode* effectNode)
{
    if (!effectNode)
    {
        Logger::Warning("ParticlesEditorController::UnregiserParticleEffectNode(): node is NULL!");
        return;
    }

    PARTICLESEFFECTITER iter = this->particleEffectNodes.find(effectNode);
    if (iter == this->particleEffectNodes.end())
    {
        Logger::Warning("ParticlesEditorController::UnregiserParticleEffectNode(): node is not found!");
        return;
    }

    this->particleEffectNodes.erase(iter);
    SAFE_DELETE(iter->second);
}

void ParticlesEditorController::Cleanup()
{
    // TODO! IMPLEMENT!
}

bool ParticlesEditorController::IsBelongToParticlesEditor(SceneGraphItem* sceneGraphItem)
{
    ExtraUserData* extraUserData = sceneGraphItem->GetExtraUserData();
    if (!extraUserData)
    {
        return false;
    }

    // Whether the node belongs to Particle Editor?
    BaseParticleEditorNode* baseParticleEditorNode = dynamic_cast<BaseParticleEditorNode*>(extraUserData);
    return (baseParticleEditorNode != NULL);
}

bool ParticlesEditorController::ShouldDisplayPropertiesInSceneEditor(SceneGraphItem *sceneGraphItem)
{
    ExtraUserData* extraUserData = sceneGraphItem->GetExtraUserData();
    if (!extraUserData)
    {
        // Non-Particle Editor at all.
        return true;
    }

    if (dynamic_cast<EffectParticleEditorNode*>(extraUserData))
    {
        // This is Particle Effect node, it has properties.
        return true;
    }
    if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        // This is Particle Emitter node, it has properties.
        return true;
    }

    return false;
}

EffectParticleEditorNode* ParticlesEditorController::GetRootForParticleEffectNode(ParticleEffectNode* effectNode)
{
    PARTICLESEFFECTITER iter = this->particleEffectNodes.find(effectNode);
    if (iter == this->particleEffectNodes.end())
    {
        return NULL;
    }
    
    return iter->second;
}

void ParticlesEditorController::SetSelectedNode(SceneGraphItem* selectedItem, bool isEmitEvent)
{
    if (IsBelongToParticlesEditor(selectedItem) == false)
    {
        Logger::Warning("ParticlesEditorController::SetSelectedNode(): attempt to select wrong node!");
        return;
    }
    
    this->selectedNode = dynamic_cast<BaseParticleEditorNode*>(selectedItem->GetExtraUserData());
    if (isEmitEvent)
    {
        EmitSelectedNodeChanged();
    }
}

void ParticlesEditorController::CleanupSelectedNode()
{
    this->selectedNode = NULL;
    EmitSelectedNodeChanged();
}

void ParticlesEditorController::EmitSelectedNodeChanged()
{
    if (this->selectedNode == NULL)
    {
        emit EmitterSelected(NULL);
        return;
    }

    // Determine the exact node type and emit the event needed.
    EmitterParticleEditorNode* emitterEditorNode = dynamic_cast<EmitterParticleEditorNode*>(this->selectedNode);
    if (emitterEditorNode)
    {
        emit EmitterSelected(emitterEditorNode->GetEmitterNode());
        return;
    }
    
    LayerParticleEditorNode* layerEditorNode = dynamic_cast<LayerParticleEditorNode*>(this->selectedNode);
    if (layerEditorNode)
    {
        emit LayerSelected(layerEditorNode->GetEmitterNode(), layerEditorNode->GetLayer());
        return;
    }
    
    ForceParticleEditorNode* forceEditorNode = dynamic_cast<ForceParticleEditorNode*>(this->selectedNode);
    if (forceEditorNode)
    {
        emit ForceSelected(forceEditorNode->GetEmitterNode(), forceEditorNode->GetLayer(),
                           forceEditorNode->GetForceIndex());
        return;
    }

    // Cleanip the selection in case we don't know what to do.
    Logger::Warning("ParticlesEditorController::EmitSelectedNodeChanged() - unknown selected node type!");
    EmitterSelected(NULL);
}

void ParticlesEditorController::AddParticleEmitterNodeToScene(ParticleEmitterNode* emitterSceneNode)
{
    // We are adding new Emitter to the Particle Effect node just selected.
    ParticleEffectNode* effectNode = NULL;
    BaseParticleEditorNode* selectedNode = GetSelectedNode();
    if (selectedNode)
    {
        effectNode = selectedNode->GetRootNode();
    }
    
    EffectParticleEditorNode* effectEditorNode = GetRootForParticleEffectNode(effectNode);
    if (effectNode && effectEditorNode)
    {
        EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(effectNode, emitterSceneNode,
                                                                                     QString::fromStdString(emitterSceneNode->GetName()));
        effectNode->AddNode(emitterSceneNode);
        effectEditorNode->AddChildNode(emitterEditorNode);
    }
}

void ParticlesEditorController::RemoveParticleEmitterNode(ParticleEmitterNode* emitterSceneNode)
{
    // Lookup for such node.
    EffectParticleEditorNode* effectEditorNode = NULL;
    EmitterParticleEditorNode* emitterEditorNode = NULL;

    FindEmitterEditorNode(emitterSceneNode, &effectEditorNode, &emitterEditorNode);

    if (effectEditorNode && emitterEditorNode)
    {
        effectEditorNode->RemoveChildNode(emitterEditorNode);
    }
}

LayerParticleEditorNode* ParticlesEditorController::AddParticleLayerToNode(EmitterParticleEditorNode* emitterNode)
{
    if (!emitterNode)
    {
        return NULL;
    }
    
    ParticleEmitter* emitter = emitterNode->GetEmitterNode()->GetEmitter();
    if (!emitter)
    {
        return NULL;
    }
    
    // Create the new layer.
    ParticleLayer *layer;
    if(emitter->GetIs3D())
    {
        layer = new ParticleLayer3D();
    }
    else
    {
        layer = new ParticleLayer();
    }

    layer->endTime = 100000000.0f;
    emitter->AddLayer(layer);

    // Create the new node and add it to the tree.
    LayerParticleEditorNode* layerNode = new LayerParticleEditorNode(emitterNode, layer);
    emitterNode->AddChildNode(layerNode);

    // Update the names for the layers
    emitterNode->UpdateLayerNames();
    SafeRelease(layer);

    return layerNode;
}

LayerParticleEditorNode* ParticlesEditorController::CloneParticleLayerNode(LayerParticleEditorNode* layerToClone)
{
    if (!layerToClone || !layerToClone->GetLayer())
    {
        return NULL;
    }
    
    EmitterParticleEditorNode* emitterNode = layerToClone->GetEmitterEditorNode();
    if (!emitterNode)
    {
        return NULL;
    }

    ParticleEmitter* emitter = emitterNode->GetEmitterNode()->GetEmitter();
    if (!emitter)
    {
        return NULL;
    }

    ParticleLayer* clonedLayer = layerToClone->GetLayer()->Clone();
    emitter->AddLayer(clonedLayer);
    
    LayerParticleEditorNode* clonedEditorNode = new LayerParticleEditorNode(emitterNode, clonedLayer);
    emitterNode->AddChildNode(clonedEditorNode);
    
    // Update the names for the layers
    emitterNode->UpdateLayerNames();
    
    return clonedEditorNode;
}

void ParticlesEditorController::RemoveParticleLayerNode(LayerParticleEditorNode* layerToRemove)
{
    if (!layerToRemove)
    {
        return;
    }
    
    EmitterParticleEditorNode* emitterNode = layerToRemove->GetEmitterEditorNode();
    if (!emitterNode)
    {
        return;
    }

    ParticleEmitter* emitter = emitterNode->GetEmitterNode()->GetEmitter();
    if (!emitter)
    {
        return;
    }
    
    // Lookup for the layer to be removed.
    int32 layerIndex = layerToRemove->GetLayerIndex();
    if (layerIndex == -1)
    {
        return;
    }
   
    // Remove the node from the layers list and also from the emitter.
    Vector<ParticleLayer*>& layers = emitter->GetLayers();
    layers.erase(layers.begin() + layerIndex);
    emitterNode->RemoveChildNode(layerToRemove);

    // Update the names for the layers
    emitterNode->UpdateLayerNames();
}

ForceParticleEditorNode* ParticlesEditorController::AddParticleForceToNode(LayerParticleEditorNode* layerNode)
{
    if (!layerNode)
    {
        return NULL;
    }
    
    ParticleLayer* layer = layerNode->GetLayer();
    if (!layer)
    {
        return NULL;
    }
    
    // Add the new Force to the Layer.
    layer->forces.push_back(RefPtr<PropertyLine<Vector3> >(new PropertyLineValue<Vector3>(Vector3(0, 0, 0))));
    layer->forcesVariation.push_back(RefPtr<PropertyLine<Vector3> >(new PropertyLineValue<Vector3>(Vector3(0, 0, 0))));
    layer->forcesOverLife.push_back(RefPtr<PropertyLine<float32> >(new PropertyLineValue<float32>(1.0f)));
    
    // Create the node for the new layer.
    int newLayerIndex = layer->forces.size() - 1;
    ForceParticleEditorNode* forceNode = new ForceParticleEditorNode(layerNode, newLayerIndex);
    layerNode->AddChildNode(forceNode);
    
    // Update the names for the forces.
    layerNode->UpdateForcesIndices();
    
    return forceNode;
}

void ParticlesEditorController::RemoveParticleForceNode(ForceParticleEditorNode* forceNode)
{
    if (!forceNode || !forceNode->GetLayerEditorNode())
    {
        return;
    }
    
    LayerParticleEditorNode* layerNode = forceNode->GetLayerEditorNode();
    ParticleLayer* layer = layerNode->GetLayer();
    if (!layer)
    {
        return;
    }

    // Remove the force from the emitter...
    int forceIndex = forceNode->GetForceIndex();
    layer->forces.erase(layer->forces.begin() + forceIndex);
    layer->forcesOverLife.erase(layer->forcesOverLife.begin() + forceIndex);
    
    // ...and from the tree.
    layerNode->RemoveChildNode(forceNode);
    
    // Done removing, recalculate the indices and names.
    layerNode->UpdateForcesIndices();
}

void ParticlesEditorController::FindEmitterEditorNode(ParticleEmitterNode* emitterSceneNode,
                                                      EffectParticleEditorNode** effectEditorNode,
                                                      EmitterParticleEditorNode** emitterEditorNode)
{
    for (PARTICLESEFFECTITER iter = particleEffectNodes.begin(); iter != particleEffectNodes.end();
         iter ++)
    {
        const BaseParticleEditorNode::PARTICLEEDITORNODESLIST& emitterEditorNodes = iter->second->GetChildren();
        for (BaseParticleEditorNode::PARTICLEEDITORNODESLISTCONSTITER innerIter = emitterEditorNodes.begin();
             innerIter != emitterEditorNodes.end(); innerIter ++)
        {
            EmitterParticleEditorNode* innerNode = dynamic_cast<EmitterParticleEditorNode*>(*innerIter);
            if (innerNode && innerNode->GetEmitterNode() == emitterSceneNode)
            {
                *effectEditorNode = iter->second;
                *emitterEditorNode = innerNode;
                break;
            }
        }
        
        // If the emitter editor found during inner loop - break the outer one too.
        if (emitterEditorNode)
        {
            break;
        }
    }
}
