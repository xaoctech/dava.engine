//
//  ParticlesEditorController.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__ParticlesEditorController__
#define __ResourceEditorQt__ParticlesEditorController__

#include "DAVAEngine.h"
#include "Base/Singleton.h"

#include "Main/SceneGraphItem.h"
#include "Scene3D/ParticleEffectNode.h"

#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EffectParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/ForceParticleEditorNode.h"

#include <QObject>

namespace DAVA {
    
// Particles Editor Controller.
class ParticlesEditorController : public QObject, public Singleton<ParticlesEditorController>
{
    Q_OBJECT
public:
    explicit ParticlesEditorController(QObject* parent = 0);
    virtual ~ParticlesEditorController();
    
    // Register/remove the Patricles Effect node.
    EffectParticleEditorNode* RegisterParticleEffectNode(ParticleEffectNode* effectNode, bool autoStart = true);
    void UnregiserParticleEffectNode(ParticleEffectNode* effectNode);

    // Whether the node belongs to Particle Editor?
    bool IsBelongToParticlesEditor(SceneGraphItem* sceneGraphItem);
    
    // For some inner nodes (like Particle Emitter one) we have to display
    // its properties in the Scene Editor. This method decides for which exactly.
    bool ShouldDisplayPropertiesInSceneEditor(SceneGraphItem *sceneGraphItem);

    // Work with selected node.
    void SetSelectedNode(SceneGraphItem* selectedItem, bool isEmitEvent);
    BaseParticleEditorNode* GetSelectedNode() const {return selectedNode;};
    void CleanupSelectedNode();

    // Get the Root node for the Particle Effect registered.
    EffectParticleEditorNode* GetRootForParticleEffectNode(ParticleEffectNode* effectNode);
    
    // Add/remove different types of Particles Editor nodes to the scene.
    void AddParticleEmitterNodeToScene(ParticleEmitterNode* emitterSceneNode);
    void RemoveParticleEmitterNode(ParticleEmitterNode* emitterSceneNode);
    
    // Cleanup the Editor nodes of the node.
    void CleanupParticleEmitterEditorNode(EmitterParticleEditorNode* emitterNode);
    
    LayerParticleEditorNode* AddParticleLayerToNode(EmitterParticleEditorNode* emitterNode);
    void RemoveParticleLayerNode(LayerParticleEditorNode* layerToRemove);
    LayerParticleEditorNode* CloneParticleLayerNode(LayerParticleEditorNode* layerToClone);

    ForceParticleEditorNode* AddParticleForceToNode(LayerParticleEditorNode* layerNode);
    void RemoveParticleForceNode(ForceParticleEditorNode* forceNode);

signals:
    void EmitterSelected(ParticleEmitterNode* emitterNode);
    void LayerSelected(ParticleEmitterNode* emitterNode, ParticleLayer* layer);
    void ForceSelected(ParticleEmitterNode* emitterNode, ParticleLayer* layer, int32 forceIndex);

protected:
    // Emit the selected node changed.
    void EmitSelectedNodeChanged();

	// Find the Emitter Editor node by the appropriate Scene Node.
    void FindEmitterEditorNode(ParticleEmitterNode* emitterSceneNode,
                               EffectParticleEditorNode** rootEditorNode,
                               EmitterParticleEditorNode** emitterEditorNode);

    // Cleanup the memory used.
    void Cleanup();

    // Particle Effects registered in the system.
    typedef Map<ParticleEffectNode*, EffectParticleEditorNode*> PARTICLESEFFECTMAP;
    typedef PARTICLESEFFECTMAP::iterator PARTICLESEFFECTITER;
    
    PARTICLESEFFECTMAP particleEffectNodes;

    // Selected node.
    BaseParticleEditorNode* selectedNode;
};
    
}

#endif /* defined(__ResourceEditorQt__ParticlesEditorController__) */
