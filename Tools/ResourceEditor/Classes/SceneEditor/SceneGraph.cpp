#include "SceneGraph.h"
#include "ControlsFactory.h"

#include "../EditorScene.h"
#include "EditorSettings.h"
#include "PropertyControlCreator.h"

#include "SceneValidator.h"

#if defined (DAVA_QT)
#include "../Qt/SceneData.h"
#include "../Qt/SceneDataManager.h"
#endif //#if defined (DAVA_QT)


SceneGraph::SceneGraph(GraphBaseDelegate *newDelegate, const Rect &rect)
    :   GraphBase(newDelegate, rect)
    ,   workingNode(NULL)
{
    CreateGraphPanel(rect);
}

SceneGraph::~SceneGraph()
{
}

void SceneGraph::SelectNode(BaseObject *node)
{
    workingNode = dynamic_cast<SceneNode *>(node);
    
#if !defined(DAVA_QT)
    if(workingNode)
    {
        List<void *> nodesForSearch;
        
        SceneNode *nd = workingNode;
        SceneNode *topSolidNode = NULL;
        while(nd)   //find solid node
        {
            if(nd->GetSolid())
            {
                topSolidNode = nd;
            }
            nd = nd->GetParent();
        }
        
        if(topSolidNode)
        {
            workingNode = topSolidNode;
            nd = topSolidNode;
        }
        else
        {
            nd = workingNode;
        }
        
        while(nd)   // fill list of nodes
        {
            nodesForSearch.push_front(nd);
            nd = nd->GetParent();
        }
        
        graphTree->OpenNodes(nodesForSearch);
        graphTree->ScrollToData(workingNode);
    }
    else
    {
        RefreshGraph();
    }
    
#endif //#if !defined(DAVA_QT)

    UpdatePropertyPanel();
}


void SceneGraph::CreateGraphPanel(const Rect &rect)
{
    GraphBase::CreateGraphPanel(rect);
    
    Rect graphRect = graphPanel->GetRect();
    graphRect.dy -= (ControlsFactory::BUTTON_HEIGHT * 7);
    graphTree = new UIHierarchy(graphRect);
    ControlsFactory::CusomizeListControl(graphTree);
    ControlsFactory::SetScrollbar(graphTree);
    graphTree->SetCellHeight(ControlsFactory::CELL_HEIGHT);
    graphTree->SetDelegate(this);
    graphTree->SetClipContents(true);
    graphPanel->AddControl(graphTree);
    
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 y = graphRect.dy;
    
    UIButton * removeRootNodes = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                                       LocalizedString(L"scenegraph.removerootnode"));
    y += ControlsFactory::BUTTON_HEIGHT;
    
    UIButton * refreshSceneGraphButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                                       LocalizedString(L"panel.refresh"));
    y += ControlsFactory::BUTTON_HEIGHT;
    
    UIButton * lookAtButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth,ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.lookatobject"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * removeNodeButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                LocalizedString(L"scenegraph.removeobject"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * enableDebugFlagsButton = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                                      LocalizedString(L"scenegraph.debugflags"));
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * bakeMatrices = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.bakematrices"));
    
    y += ControlsFactory::BUTTON_HEIGHT;
    UIButton * buildQuadTree = ControlsFactory::CreateButton(Rect(0, y, leftSideWidth, ControlsFactory::BUTTON_HEIGHT), 
                                                            LocalizedString(L"scenegraph.buildquadtree"));
    
#if !defined (DAVA_QT)
    removeRootNodes->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnRemoveRootNodesButtonPressed));
    refreshSceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnRefreshGraph));
    lookAtButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnLookAtButtonPressed));
    removeNodeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnRemoveNodeButtonPressed));
    enableDebugFlagsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnEnableDebugFlagsPressed));
    bakeMatrices->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnBakeMatricesPressed));
    buildQuadTree->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnBuildQuadTreePressed));
#endif //#if !defined (DAVA_QT)
    
    graphPanel->AddControl(removeRootNodes);
    graphPanel->AddControl(refreshSceneGraphButton);
    graphPanel->AddControl(lookAtButton);
    graphPanel->AddControl(removeNodeButton);
    graphPanel->AddControl(enableDebugFlagsButton);
    graphPanel->AddControl(bakeMatrices);
    graphPanel->AddControl(buildQuadTree);
    
    SafeRelease(removeRootNodes);
    SafeRelease(refreshSceneGraphButton);
    SafeRelease(lookAtButton);
    SafeRelease(removeNodeButton);
    SafeRelease(enableDebugFlagsButton);
    SafeRelease(bakeMatrices);
    SafeRelease(buildQuadTree);
}

void SceneGraph::FillCell(UIHierarchyCell *cell, void *node)
{
    SceneNode *n = (SceneNode *)node;
    UIStaticText *text =  (UIStaticText *)cell->FindByName("_Text_");
    text->SetText(StringToWString(n->GetName()));
    
    UIControl *icon = cell->FindByName("_Icon_");
    icon->SetSprite("~res:/Gfx/UI/SceneNode/scenenode", 0);
    
    UIControl *marker = cell->FindByName("_Marker_");
    if(n->GetFlags() & SceneNode::NODE_INVALID)
    {
        marker->SetSprite("~res:/Gfx/UI/SceneNode/scenenode_invalid", 0);
        marker->SetVisible(true);
    }
    else 
    {
        marker->SetVisible(false);
    }
    
    
    if(n == workingNode)
    {
        cell->SetSelected(true, false);
    }
    else
    {
        cell->SetSelected(false, false);
    }
}

void SceneGraph::SelectHierarchyNode(UIHierarchyNode * node)
{
    SceneNode * sceneNode = dynamic_cast<SceneNode*>((BaseObject*)node->GetUserNode());
    if (sceneNode)
    {
        workingNode = sceneNode;
        
        workingScene->SetSelection(0);
        workingScene->SetSelection(workingNode);
        
        UpdatePropertyPanel();
        
        Camera * cam = dynamic_cast<Camera*>(workingNode);
        if (cam)
        {
            if (InputSystem::Instance()->GetKeyboard()->IsKeyPressed(DVKEY_ALT))
            {
                workingScene->SetClipCamera(cam);
            }
            else 
            {
                workingScene->SetCurrentCamera(cam);
            }
        }
    }

}


void SceneGraph::UpdatePropertyPanel()
{
    if(workingNode && (NULL != graphTree->GetParent()))
    {
		RecreatePropertiesPanelForNode(workingNode);
        
        if(propertyControl->GetParent() && propertyControl->GetParent() != propertyPanel)
        {
            propertyControl->GetParent()->RemoveControl(propertyControl);
        }
        
        if(!propertyControl->GetParent())
        {
            propertyPanel->AddControl(propertyControl);
        }
        RefreshProperties();
    }
    else
    {
        if(propertyControl && propertyControl->GetParent())
        {
            propertyPanel->RemoveControl(propertyControl);
        }
    }
}

void SceneGraph::RecreatePropertiesPanelForNode(SceneNode * node)
{
	if(propertyControl && propertyControl->GetParent())
	{
		propertyPanel->RemoveControl(propertyControl);
	}
	SafeRelease(propertyControl);
    
    DVASSERT(delegate);
    if(delegate->LandscapeEditorActive())
    {
        propertyControl = delegate->GetPropertyControl(propertyPanelRect);
        //PropertyControlCreator::Instance()->CreateControlForLandscapeEditor(node, propertyPanelRect);
    }
    else 
    {
        propertyControl = PropertyControlCreator::Instance()->CreateControlForNode(node, propertyPanelRect, false);
    }
    
    SafeRetain(propertyControl);
	propertyControl->SetDelegate(this);
	propertyControl->SetWorkingScene(workingScene);
    propertyControl->ReadFrom(node);
}


void SceneGraph::OnRemoveNodeButtonPressed(BaseObject *, void *, void *)
{
#if defined(DAVA_QT)
    SceneData *activeScene = SceneDataManager::Instance()->GetActiveScene();
    activeScene->RemoveSceneNode(workingNode);
#else //#if defined(DAVA_QT)
    RemoveWorkingNode();
#endif //#if defined(DAVA_QT)
    
}

void SceneGraph::OnRemoveRootNodesButtonPressed(BaseObject *, void *, void *)
{
    RemoveRootNodes();
}


void SceneGraph::OnLookAtButtonPressed(BaseObject *, void *, void *)
{
    MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode*>(workingNode);
    if (mesh)
    {
        AABBox3 bbox = mesh->GetBoundingBox();
        AABBox3 transformedBox;
        bbox.GetTransformedBox(mesh->GetWorldTransform(), transformedBox);
        Vector3 center = transformedBox.GetCenter();
        workingScene->GetCurrentCamera()->SetTarget(center);
    }
}

void SceneGraph::OnBakeMatricesPressed(BaseObject *, void *, void *)
{
    if (workingNode)
    {
        workingNode->BakeTransforms();
    }
// TODO: if node is not selected scene should be selected by default ??? or probably we should show 
//    else
//    {
//        scene->BakeTransforms(); 
//    }
}

void SceneGraph::OnBuildQuadTreePressed(BaseObject *, void *, void *)
{
    if (workingNode)
    {
//        Scene * scene = workingNode->GetScene();
//        QuadTree * quadTree = dynamic_cast<QuadTree*>(scene->GetBVHierarchy());
//        if (!quadTree)
//        {
//            quadTree = new QuadTree();
//            scene->SetBVHierarchy(quadTree);
//        }
//        quadTree->Build(scene);
    }
}

void SceneGraph::OnEnableDebugFlagsPressed(BaseObject *, void *, void *)
{
    if (workingNode)
    {
        if (workingNode->GetDebugFlags() & SceneNode::DEBUG_DRAW_ALL)
        {
            workingNode->SetDebugFlags(0, true);
        }
        else
        {
            workingNode->SetDebugFlags(SceneNode::DEBUG_DRAW_ALL, true);
        }
    }
}


void SceneGraph::OnRefreshGraph(BaseObject *, void *, void *)
{
    RefreshGraph();
}

void SceneGraph::RemoveWorkingNode()
{
    if (workingNode)
    {
        SceneNode * parentNode = workingNode->GetParent();
        if (parentNode)
        {
			workingScene->ReleaseUserData(workingNode);
			workingScene->SetSelection(0);

			SceneNode * tempNode = SafeRetain(workingNode);
            parentNode->RemoveNode(workingNode);
            workingNode = NULL;

            UpdatePropertyPanel();

			SafeRelease(tempNode);
            
            graphTree->Refresh();
            
            SceneValidator::Instance()->EnumerateSceneTextures();
        }
    }
}

void SceneGraph::RemoveRootNodes()
{
    if (workingNode && workingScene)
    {
        if(workingNode->GetParent() == workingScene) 
        {
            String referenceToOwner;
            
            KeyedArchive *customProperties = workingNode->GetCustomProperties();
            if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
            {
                referenceToOwner = customProperties->GetString("editor.referenceToOwner");
            }

            
            Vector<SceneNode *>nodesForDeletion;
            nodesForDeletion.reserve(workingScene->GetChildrenCount());
            
            for(int32 i = 0; i < workingScene->GetChildrenCount(); ++i)
            {
                SceneNode *node = workingScene->GetChild(i);

                customProperties = node->GetCustomProperties();
                if(customProperties && customProperties->IsKeyExists("editor.referenceToOwner"))
                {
                    if(customProperties->GetString("editor.referenceToOwner") == referenceToOwner)
                    {
                        nodesForDeletion.push_back(SafeRetain(node));
                    }
                }
            }
            
            workingScene->SetSelection(NULL);
            workingNode = NULL;
            for(int32 i = 0; i < nodesForDeletion.size(); ++i)
            {
                SceneNode *node = nodesForDeletion[i];
                
                workingScene->ReleaseUserData(node);
                workingScene->RemoveNode(node);
                
                SafeRelease(node);
            }
            nodesForDeletion.clear();
            
            UpdatePropertyPanel();
            graphTree->Refresh();
            SceneValidator::Instance()->EnumerateSceneTextures();
        }
    }
}


#pragma mark --UIHierarchyDelegate
bool SceneGraph::IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode)
{
    if (forNode) 
    {
        SceneNode *node = (SceneNode*)forNode;
        if(node->GetSolid())
        {
            return false;
        }
        else
        {
            return node->GetChildrenCount() > 0;
        }
    }
    
    return workingScene->GetChildrenCount() > 0;
}

int32 SceneGraph::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if (forParent) 
    {
        SceneNode *node = (SceneNode*)forParent;
        if(node->GetSolid())
        {
            return 0;
        }
        else
        {
            return node->GetChildrenCount();
        }
        
    }
    
    if(workingScene)    return workingScene->GetChildrenCount();

    return 0;
}

void * SceneGraph::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
    if (forParent) 
    {
        return ((SceneNode*)forParent)->GetChild(index);
    }
    
    return workingScene->GetChild(index);
}

void SceneGraph::DragAndDrop(void *who, void *target, int32 mode)
{
    SceneNode *whoNode = SafeRetain((SceneNode *)who);
    SceneNode *targetNode = SafeRetain((SceneNode *)target);
    
    if(whoNode)
    {
        if(UIHierarchy::DRAG_CHANGE_PARENT == mode)
        {
            // select new parent for dragged node
            SceneNode *newParent = (targetNode) ? targetNode : workingScene;
            
            //skip unused drag
            if(whoNode->GetParent() != newParent)
            {
                // check correct hierarhy (can't drag to child)
                SceneNode *nd = newParent->GetParent();
                while(nd && nd != whoNode)
                {
                    nd = nd->GetParent();
                }
                
                if(!nd)
                {
                    //drag
                    whoNode->GetParent()->RemoveNode(whoNode);
                    newParent->AddNode(whoNode);
                }
            }
        }
        else if(UIHierarchy::DRAG_CHANGE_ORDER == mode)
        {
            if(targetNode && whoNode->GetParent() == targetNode->GetParent())
            {
                whoNode->GetParent()->RemoveNode(whoNode);
                targetNode->GetParent()->InsertBeforeNode(whoNode, targetNode);
            }
        }

        SelectNode(NULL);
    }
    
    SafeRelease(whoNode);
    SafeRelease(targetNode);
}

void SceneGraph::RefreshGraph()
{
    GraphBase::RefreshGraph();
    
    SceneValidator::Instance()->EnumerateNodes(workingScene);
}
