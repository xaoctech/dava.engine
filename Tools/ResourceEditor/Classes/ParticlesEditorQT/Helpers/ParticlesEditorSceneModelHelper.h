//
//  ParticlesEditorSceneHelper.h
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#ifndef __ResourceEditorQt__ParticlesEditorSceneHelper__
#define __ResourceEditorQt__ParticlesEditorSceneHelper__

#include "DAVAEngine.h"
#include <QItemSelection>
#include <QModelIndex>
#include <QMenu>

#include "ParticlesEditorQT/Nodes/BaseParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EffectParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/EmitterParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"
#include "ParticlesEditorQT/Nodes/ForceParticleEditorNode.h"

#include "Scene/SceneData.h"
#include "Main/SceneGraphItem.h"

namespace DAVA {

// Scene Helper for Particles Editor.
class ParticlesEditorSceneHelper
{
public:
    // Custom processing the Selection Changed in the Scene Graph model. Returns
    // TRUE if no further processing needed.
    bool ProcessSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    // Custom node selection.
    SceneGraphItem* GetGraphItemToBeSelected(GraphItem* rootItem, SceneNode* node);

    // Preprocess the Scene Node during adding, change its type if needed.
    SceneNode* PreprocessSceneNode(SceneNode* rawNode);

    // Add the new node, if it is related to Particles Editor. Returns TRUE if
    // no further processing needed.
    bool AddSceneNode(SceneNode* node) const;
    
    // Remove the Scene Node, if it is related to Particles Editor.
    void RemoveSceneNode(SceneNode* node) const;
    
    // Add the node and all its children to the Scene Graph.
    bool AddNodeToSceneGraph(SceneGraphItem *graphItem, SceneNode *node);

    // Whether we need to display "standard" Scene Editor Popup Menu for this node?
    bool NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const;

    // Add Popup Menu items depending on the tree node selected.
    void AddPopupMenuItems(SceneData* sceneData, QMenu &menu, const QModelIndex &index) const;
    
protected:
    // Build the Scene Graph in a recursive way.
    void BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem);

    // Determine whether we need to select Editor Tree Node in a recursive way.
    SceneGraphItem* GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, SceneNode* node);

    // Do the checks needed and return ExtraUserData from model index.
    ExtraUserData* GetExtraUserDataByModelIndex(const QModelIndex& modelIndex) const;
    
    // Synchronization of the whole Particle Editor Tree and different types of Nodes.
    void SynchronizeParticleEditorTree(BaseParticleEditorNode* node);

    void SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node, ParticleEffectNode* effectRootNode);
    void SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node);
    void SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node);

};
}

#endif /* defined(__ResourceEditorQt__ParticlesEditorSceneHelper__) */
