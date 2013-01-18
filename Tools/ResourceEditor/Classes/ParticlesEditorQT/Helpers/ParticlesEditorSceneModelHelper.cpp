//
//  ParticlesEditorSceneModelHelper.cpp
//  ResourceEditorQt
//
//  Created by Yuri Coder on 11/26/12.
//
//

#include "ParticlesEditorSceneModelHelper.h"
#include "DockParticleEditor/ParticlesEditorController.h"

#include "Commands/SceneGraphCommands.h"
#include "Commands/ParticleEditorCommands.h"

#include "Entity/Component.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

// Custom processing the Selection Changed in the Scene Graph model. Returns
// TRUE if no further processing needed.
bool ParticlesEditorSceneModelHelper::ProcessSelectionChanged(const QItemSelection &selected,
                                                         const QItemSelection &deselected)
{
    int32 deselectedSize = deselected.size();
    int32 selectedSize = selected.size();

    if (selectedSize <= 0)
    {
        // De-selection happened - allow processing by caller.
        ParticlesEditorController::Instance()->CleanupSelectedNode();
        return false;
    }
    
    // Determine whether the selected node belongs to Particle Editor.
    QItemSelectionRange selectedRange = selected.at(0);
    SceneGraphItem *selectedItem = static_cast<SceneGraphItem*>(selectedRange.topLeft().internalPointer());

    if (ParticlesEditorController::Instance()->IsBelongToParticlesEditor(selectedItem))
    {
        // Our one. Select and emit the appropriate event.
        ParticlesEditorController::Instance()->SetSelectedNode(selectedItem, true);
        return !ParticlesEditorController::Instance()->ShouldDisplayPropertiesInSceneEditor(selectedItem);
    }

    // Not ours. Cleanup the selected node and return control back to the caller.
    ParticlesEditorController::Instance()->CleanupSelectedNode();
    return false;
}

// Custom node selection.
SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemToBeSelected(GraphItem* rootItem, SceneNode* node)
{
    // Do we have something marked for selection on the Particles Editor? If yes,
    // do select it.
    return GetGraphItemToBeSelectedRecursive(rootItem, node);
    
    return false;
}

SceneGraphItem* ParticlesEditorSceneModelHelper::GetGraphItemToBeSelectedRecursive(GraphItem* rootItem, SceneNode* node)
{
    SceneGraphItem* graphRootItem = dynamic_cast<SceneGraphItem*>(rootItem);
    if (!graphRootItem || graphRootItem->GetExtraUserData() == NULL)
    {
        return NULL;
    }
    
    BaseParticleEditorNode* editorNode = dynamic_cast<BaseParticleEditorNode*>(graphRootItem->GetExtraUserData());
    if (!editorNode)
    {
        return NULL;
    }
    
    if (editorNode->IsMarkedForSelection())
    {
        editorNode->SetMarkedToSelection(false);
        return graphRootItem;
    }
    
    // Verify all the child nodes for the Scene Graph.
    int32 childCount = rootItem->ChildrenCount();
    for (int32 i = 0; i < childCount; i ++)
    {
        SceneGraphItem* childGraphItem = dynamic_cast<SceneGraphItem*>(rootItem->Child(i));
        SceneGraphItem* graphItemToBeSelected = GetGraphItemToBeSelectedRecursive(childGraphItem, node);
        if (graphItemToBeSelected)
        {
            return graphItemToBeSelected;
        }
    }

    return NULL;
}

SceneNode* ParticlesEditorSceneModelHelper::PreprocessSceneNode(SceneNode* rawNode)
{
    // There is one and only preprocessing case - if the "raw" node is "orphaned" Particles Emitter
    // (without the ParticlesEffectNode parent), we'll create the new Particles Effect node and
    // move the raw Emitter node to it.
    
	ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(rawNode->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
    if (!emitterComponent)
    {
        return rawNode;
    }

	//ParticleEmitterNode* emitterNode = static_cast<ParticleEmitterNode*>(rawNode);
	//emitterNode->SetEmitter(emitterComponent->GetParticleEmitter());
    SceneNode* curParentNode = rawNode->GetParent();
    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(curParentNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    // If the Emitter Node has parent, but its parent is not ParticleEffectNode, we need to
    // "adopt" this node by creating ParticleEffect node and attaching.
    if (curParentNode && (effectComponent == NULL))
    {
		SceneNode* newParentNodeParticleEffect = CreateParticleEffectNode();
		curParentNode->AddNode(newParentNodeParticleEffect);

        // Add the emitter node to the new Effect (this will also remove it from the scene).
        newParentNodeParticleEffect->AddNode(rawNode);

        // Register the Particle Editor structure for the new node.
        EffectParticleEditorNode* effectEditorNode =
        ParticlesEditorController::Instance()->RegisterParticleEffectNode(newParentNodeParticleEffect);
        EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(newParentNodeParticleEffect,
            rawNode, QString::fromStdString(rawNode->GetName()));
        effectEditorNode->AddChildNode(emitterEditorNode);

        return newParentNodeParticleEffect;
    }
    
    // No preprocessing needed.
    return rawNode;
}

SceneNode* ParticlesEditorSceneModelHelper::CreateParticleEffectNode()
{
	SceneNode * newParentNodeParticleEffect = new SceneNode();
	newParentNodeParticleEffect->SetName("Particle effect");
	ParticleEffectComponent * newEffectComponent = new ParticleEffectComponent();
	newParentNodeParticleEffect->AddComponent(newEffectComponent);

	return newParentNodeParticleEffect;
}

bool ParticlesEditorSceneModelHelper::AddNodeToSceneGraph(SceneGraphItem *graphItem, SceneNode *node)
{
    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    if (effectComponent == NULL)
    {
        // Not ours - process as-is.
        return false;
    }

    EffectParticleEditorNode* effectEditorNode = ParticlesEditorController::Instance()->GetRootForParticleEffectNode(node);
    if (!effectEditorNode)
    {
        // Possible while loading projects - register the node in this case.
        effectEditorNode = ParticlesEditorController::Instance()->RegisterParticleEffectNode(node);
    }
    
    if (graphItem->GetExtraUserData() == NULL)
    {
        // Register the root. Note - it has both User Data and Extra Data.
        graphItem->SetExtraUserData(effectEditorNode);
    }

    // Build the Scene Graph in a recursive way.
    BuildSceneGraphRecursive(effectEditorNode, graphItem);
    return true;
}

void ParticlesEditorSceneModelHelper::BuildSceneGraphRecursive(BaseParticleEditorNode* rootNode, SceneGraphItem* rootItem)
{
    // Prior to buildind the tree - need to check whether the children are in sync and synchronize them,
    // if not. This is actually needed for Load() process.
    SynchronizeParticleEditorTree(rootNode);
    
    // Set the UserData for inner Emitter nodes.
    if (dynamic_cast<EmitterParticleEditorNode*>(rootNode))
    {
        SceneNode* emitterNode = (static_cast<EmitterParticleEditorNode*>(rootNode))->GetEmitterNode();
        rootItem->SetUserData(emitterNode);
    }

    int childrenCount = rootNode->GetChildren().size();
    for (List<BaseParticleEditorNode*>::const_iterator iter = rootNode->GetChildren().begin();
         iter != rootNode->GetChildren().end(); iter ++)
    {
        BaseParticleEditorNode* childNode = (*iter);

        SceneGraphItem* childItem = new SceneGraphItem();
        
        childItem->SetExtraUserData(childNode);
        rootItem->AppendChild(childItem);

        // Repeat for all children.
        BuildSceneGraphRecursive(childNode, childItem);
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeParticleEditorTree(BaseParticleEditorNode* node)
{
	EffectParticleEditorNode* effectEditorNode = dynamic_cast<EffectParticleEditorNode*>(node);
    if (effectEditorNode)
    {
		SceneNode* effectRootNode = node->GetRootNode();
        SynchronizeEffectParticleEditorNode(effectEditorNode, effectRootNode);
    }

    EmitterParticleEditorNode* emitterEditorNode = dynamic_cast<EmitterParticleEditorNode*>(node);
    if (emitterEditorNode)
    {
        SynchronizeEmitterParticleEditorNode(emitterEditorNode);
    }
    
    LayerParticleEditorNode* layerEditorNode = dynamic_cast<LayerParticleEditorNode*>(node);
    if (layerEditorNode)
    {
        SynchronizeLayerParticleEditorNode(layerEditorNode);
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeEffectParticleEditorNode(EffectParticleEditorNode* node,
																	 SceneNode* effectRootNode)
{
    if (!node || !effectRootNode)
    {
        return;
    }

    // All the children of Effect Node are actually Emitters.
    int emittersCountInEffect = effectRootNode->GetChildrenCount();
    int emittersCountInEffectNode = node->GetEmittersCount();

    if (emittersCountInEffect > 0 && emittersCountInEffectNode == 0)
    {
        // Root Node has no Emitters - need to add them.
        for (int32 i = 0; i < emittersCountInEffect; i ++)
        {
            // Create the new Emitter and add it to the tree.
			ParticleEmitterComponent * emitterComponent = cast_if_equal<ParticleEmitterComponent*>(effectRootNode->GetChild(i)->GetComponent(Component::PARTICLE_EMITTER_COMPONENT));
			if (!emitterComponent)
			{
			    continue;
			}

 
            EmitterParticleEditorNode* emitterEditorNode = new EmitterParticleEditorNode(effectRootNode, effectRootNode->GetChild(i),
                                                                                         QString::fromStdString(effectRootNode->GetChild(i)->GetName()));
            node->AddChildNode(emitterEditorNode);
        }
    }
}

void ParticlesEditorSceneModelHelper::SynchronizeEmitterParticleEditorNode(EmitterParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }

    ParticleEmitterComponent* emitterComponent = node->GetParticleEmitterComponent();
    if (!emitterComponent)
    {
        return;
    }
    
    ParticleEmitter* emitter = emitterComponent->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }

    int layersCountInEmitter = emitter->GetLayers().size();
    int layersCountInEmitterNode = node->GetLayersCount();

    if (layersCountInEmitter > 0 && layersCountInEmitterNode == 0)
    {
        // Emitter Node has no layers - need to add them.
        for (int32 i = 0; i < layersCountInEmitter; i ++)
        {
            // Create the new node and add it to the tree.
            LayerParticleEditorNode* layerNode = new LayerParticleEditorNode(node, emitter->GetLayers()[i]);
            node->AddChildNode(layerNode);
        }

        node->UpdateLayerNames();
     }
}

void ParticlesEditorSceneModelHelper::SynchronizeLayerParticleEditorNode(LayerParticleEditorNode* node)
{
    if (!node)
    {
        return;
    }
    
    ParticleEmitterComponent* emitterComponent = node->GetParticleEmitterComponent();
    if (!emitterComponent)
    {
        return;
    }
    
    ParticleEmitter* emitter = emitterComponent->GetParticleEmitter();
    if (!emitter)
    {
        return;
    }

    ParticleLayer* layer = node->GetLayer();
    if (!layer)
    {
        return;
    }

    // Synchronize the Forces.
    int32 forcesCountInLayer = layer->forces.size();
    int32 forcesCountInLayerNode = node->GetForcesCount();

    if (forcesCountInLayer > 0 && forcesCountInLayerNode == 0)
    {
        // Emitter Layer has no Forces - need to add them.
        for (int32 i = 0; i < forcesCountInLayer; i ++)
        {
            ForceParticleEditorNode* forceNode = new ForceParticleEditorNode(node, i);
            node->AddChildNode(forceNode);
        }

        node->UpdateForcesIndices();
    }
}

bool ParticlesEditorSceneModelHelper::NeedDisplaySceneEditorPopupMenuItems(const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserDataByModelIndex(index);
    if (!extraUserData)
    {
        return true;
    }
    
    // We should block Scene Editor popup menu items for all child nodes for Particle Effect Node.
    if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        return false;
    }

    if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        return false;
    }
    
    return true;
}

ExtraUserData* ParticlesEditorSceneModelHelper::GetExtraUserDataByModelIndex(const QModelIndex& modelIndex) const
{
    if (!modelIndex.isValid())
    {
        return NULL;
    }

    SceneGraphItem *item = static_cast<SceneGraphItem*>(modelIndex.internalPointer());
    if (!item)
    {
        return NULL;
    }
    
    return item->GetExtraUserData();
}

void ParticlesEditorSceneModelHelper::AddPopupMenuItems(QMenu& menu, const QModelIndex &index) const
{
    ExtraUserData* extraUserData = GetExtraUserDataByModelIndex(index);
    if (!extraUserData)
    {
        return;
    }
    
    // Which kind of Node is it?
    if (dynamic_cast<EffectParticleEditorNode*>(extraUserData))
    {
        // Effect Node. Allow to add Particle Emitters and start/stop the animation.
        AddActionToMenu(&menu, QString("Add Particle Emitter"), new CommandAddParticleEmitter());
        AddActionToMenu(&menu, QString("Start Particle Effect"), new CommandStartStopParticleEffect(true));
        AddActionToMenu(&menu, QString("Stop Particle Effect"), new CommandStartStopParticleEffect(false));
        AddActionToMenu(&menu, QString("Restart Particle Effect"), new CommandRestartParticleEffect());
    }
    else if (dynamic_cast<EmitterParticleEditorNode*>(extraUserData))
    {
        // For Particle Emitter we also allow to load/save it.
        AddActionToMenu(&menu, QString("Load Emitter from Yaml"), new CommandLoadParticleEmitterFromYaml());
        AddActionToMenu(&menu, QString("Save Emitter to Yaml"), new CommandSaveParticleEmitterToYaml(false));
        AddActionToMenu(&menu, QString("Save Emitter to Yaml As"), new CommandSaveParticleEmitterToYaml(true));
        
        // Emitter node. Allow to remove it and also add Layers.
        AddActionToMenu(&menu, QString("Remove Particle Emitter"), new CommandRemoveSceneNode());
        AddActionToMenu(&menu, QString("Add Layer"), new CommandAddParticleEmitterLayer());
    }
    else if (dynamic_cast<LayerParticleEditorNode*>(extraUserData))
    {
        // Layer Node. Allow to remove it and add new Force.
        AddActionToMenu(&menu, QString("Remove Layer"), new CommandRemoveParticleEmitterLayer());
        AddActionToMenu(&menu, QString("Clone Layer"), new CommandCloneParticleEmitterLayer());
        AddActionToMenu(&menu, QString("Add Force"), new CommandAddParticleEmitterForce());
    }
    else if (dynamic_cast<ForceParticleEditorNode*>(extraUserData))
    {
        // Force Node. Allow to remove it.
        AddActionToMenu(&menu, QString("Remove Force"), new CommandRemoveParticleEmitterForce());
    }
}

void ParticlesEditorSceneModelHelper::AddActionToMenu(QMenu *menu, const QString &actionTitle, Command *command) const
{
    QAction *action = menu->addAction(actionTitle);
    action->setData(PointerHolder<Command *>::ToQVariant(command));
}
