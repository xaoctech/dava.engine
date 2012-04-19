#include "SceneGraph.h"
#include "ControlsFactory.h"

#include "../EditorScene.h"
#include "EditorSettings.h"
#include "PropertyControlCreator.h"

#include "SceneValidator.h"


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
    
    UpdatePropertyPanel();
}


void SceneGraph::CreateGraphPanel(const Rect &rect)
{
    GraphBase::CreateGraphPanel(rect);
    
    Rect graphRect = graphPanel->GetRect();
    graphRect.dy -= (ControlsFactory::BUTTON_HEIGHT * 6);
    graphTree = new UIHierarchy(graphRect);
    ControlsFactory::CusomizeListControl(graphTree);
    ControlsFactory::SetScrollbar(graphTree);
    graphTree->SetCellHeight(ControlsFactory::CELL_HEIGHT);
    graphTree->SetDelegate(this);
    graphTree->SetClipContents(true);
    graphPanel->AddControl(graphTree);
    
    int32 leftSideWidth = EditorSettings::Instance()->GetLeftPanelWidth();
    int32 y = graphRect.dy;
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
    
    
    refreshSceneGraphButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnRefreshGraph));
    lookAtButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnLookAtButtonPressed));
    removeNodeButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnRemoveNodeButtonPressed));
    enableDebugFlagsButton->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnEnableDebugFlagsPressed));
    bakeMatrices->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnBakeMatricesPressed));
    buildQuadTree->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &SceneGraph::OnBuildQuadTreePressed));
    
    graphPanel->AddControl(refreshSceneGraphButton);
    graphPanel->AddControl(lookAtButton);
    graphPanel->AddControl(removeNodeButton);
    graphPanel->AddControl(enableDebugFlagsButton);
    graphPanel->AddControl(bakeMatrices);
    graphPanel->AddControl(buildQuadTree);
    
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
    cell->text->SetText(StringToWString(n->GetName()));
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


void SceneGraph::OnRemoveNodeButtonPressed(BaseObject * obj, void *, void *)
{
    RemoveWorkingNode();
}

void SceneGraph::OnLookAtButtonPressed(BaseObject * obj, void *, void *)
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

void SceneGraph::OnBakeMatricesPressed(BaseObject * obj, void *, void *)
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

void SceneGraph::OnBuildQuadTreePressed(BaseObject * obj, void *, void *)
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

void SceneGraph::OnEnableDebugFlagsPressed(BaseObject * obj, void *, void *)
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


void SceneGraph::OnRefreshGraph(BaseObject * obj, void *, void *)
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
            parentNode->RemoveNode(workingNode);
            
            workingNode = NULL;
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
    
    return workingScene->GetChildrenCount();
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


