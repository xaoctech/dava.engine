/*
 *  EditorScene.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "EditorScene.h"
#include "SceneNodeUserData.h"
#include "SceneValidator.h"


/*
    This means that if we'll call GameScene->GetClassName() it'll return "Scene"
    This is for correct serialization of framework subclasses.
 */
REGISTER_CLASS_WITH_ALIAS(EditorScene, "Scene");

EditorScene::EditorScene()
:Scene()
{ 
	selection = 0;
	lastSelectedPhysics = 0;
	proxy = 0;
    btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
        
    collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	collisionWorld = new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);

//    RegisterLodLayer(0, 15);
//    RegisterLodLayer(12, 35);
//    RegisterLodLayer(31, 1000);
    RegisterLodLayer(0, 10);
    RegisterLodLayer(7, 20);
    RegisterLodLayer(18, 1000);
}

EditorScene::~EditorScene()
{
	ReleaseUserData(this);
	SafeDelete(collisionWorld);
	SafeDelete(broadphase);
	SafeDelete(dispatcher);
	SafeDelete(collisionConfiguration);
}

void EditorScene::Update(float32 timeElapsed)
{    
    Scene::Update(timeElapsed);
	CheckNodes(this);
	collisionWorld->updateAabbs();
}

void EditorScene::CheckNodes(SceneNode * curr)
{
	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);	
	
	if (mesh && mesh->GetUserData() == 0)
	{
		SceneNodeUserData * data = new SceneNodeUserData();
		curr->SetUserData(data);
		data->bulletObject = new BulletObject(this, collisionWorld, mesh, mesh->GetWorldTransform());
		SafeRelease(data);
	}
	else if (mesh && mesh->GetUserData())
	{
		SceneNodeUserData * data = (SceneNodeUserData*)mesh->GetUserData();
		data->bulletObject->UpdateCollisionObject();
	}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		CheckNodes(curr->GetChild(i));
	}
}

void EditorScene::ReleaseUserData(SceneNode * curr)
{
	if (curr)
		curr->SetUserData(0);

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		ReleaseUserData(curr->GetChild(i));
	}
}

SceneNode * GetSolidParent(SceneNode* curr)
{
	if (curr->GetSolid())
	{
		return curr;
	}
	else 
	{
		SceneNode * parent = curr->GetParent();
		if (parent == 0)
			return 0;
		return GetSolidParent(parent);
	}
}

void EditorScene::TrySelection(Vector3 from, Vector3 direction)
{
	if (selection)
		selection->SetDebugFlags(selection->GetDebugFlags() & (~SceneNode::DEBUG_DRAW_AABOX_CORNERS));

	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(direction.x, direction.y, direction.z);
		
    btCollisionWorld::AllHitsRayResultCallback cb(pos, to);
    collisionWorld->rayTest(pos, to, cb);
	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		Logger::Debug("Has Hit");
		int findedIndex = cb.m_collisionObjects.size() - 1;
		if(lastSelectedPhysics)
		{
			SceneNodeUserData * data = (SceneNodeUserData*)lastSelectedPhysics->GetUserData();
			if (data)
			{
				for (int i = cb.m_collisionObjects.size() - 1; i >= 0 ; i--)
				{					
					if (data->bulletObject->GetCollisionObject() == cb.m_collisionObjects[i])
					{
						findedIndex = i;
						break;
					}
				}
				while (findedIndex >= 0 && data->bulletObject->GetCollisionObject() == cb.m_collisionObjects[findedIndex])
					findedIndex--;
				findedIndex = findedIndex % cb.m_collisionObjects.size();
			}
		}
		Logger::Debug("size:%d selIndex:%d", cb.m_collisionObjects.size(), findedIndex);
		
		if (findedIndex == -1)
			findedIndex = cb.m_collisionObjects.size() - 1;
		coll = cb.m_collisionObjects[findedIndex];
		selection = FindSelected(this, coll);
		lastSelectedPhysics = selection;
		SetSelection(selection);
	}
	else 
	{
		SetSelection(0);
	}
}

LandscapeNode * EditorScene::GetLandScape(SceneNode *node)
{
    LandscapeNode *land = dynamic_cast<LandscapeNode *>(node);
	
    if (land) 
    {
		return land;
    }
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        SceneNode * child = node->GetChild(ci);
		LandscapeNode * result = GetLandScape(child);
		if (result)
			return result;
    }
	return 0;
}

SceneNode * EditorScene::FindSelected(SceneNode * curr, btCollisionObject * coll)
{
	SceneNode * node = dynamic_cast<MeshInstanceNode *> (curr);
	if (node == 0)
		node = dynamic_cast<LightNode *> (curr);
	
	if (node && node->GetUserData())
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->GetUserData();
		if (data->bulletObject->GetCollisionObject() == coll)
			return curr;
	}
	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		SceneNode * result = FindSelected(curr->GetChild(i), coll);
		if (result)
			return result;
	}
	return 0;
}

void EditorScene::SetSelection(SceneNode *newSelection)
{
    if (selection)
    {
		selection->SetDebugFlags(selection->GetDebugFlags() & (~SceneNode::DEBUG_DRAW_AABOX_CORNERS));
    }
    
	selection = newSelection;
	if (selection)
	{
		SceneNode * solid = GetSolidParent(selection);
		if (solid)
			selection = solid;
		
		proxy = GetHighestProxy(selection);
		if (proxy == 0)
			proxy = selection;
	}
	else
	{
		proxy = 0;
		lastSelectedPhysics = 0;
	}

	if(selection)
		selection->SetDebugFlags(selection->GetDebugFlags() | (SceneNode::DEBUG_DRAW_AABOX_CORNERS));
}

SceneNode * EditorScene::GetHighestProxy(SceneNode* curr)
{
	int32 cc = curr->GetChildrenCount();
	if (cc == 0)
		return GetHighestProxy(curr->GetParent());
	if (cc > 1)
		return 0;
	if (cc == 1)
    {
        SceneNode * result = GetHighestProxy(curr->GetParent());
	    if (result == 0)
            return curr;
        else return result;
        
    }
    return NULL;
}

void EditorScene::SetBulletUpdate(SceneNode* curr, bool value)
{
	SceneNodeUserData * userData = (SceneNodeUserData*)curr->GetUserData();
	if (userData)
		userData->bulletObject->SetUpdateFlag(value);
	
	for (int32 i = 0; i < curr->GetChildrenCount(); i++)
	{
		SetBulletUpdate(curr->GetChild(i), value);
	}
}


void EditorScene::Draw()
{
//	DrawDebugNodes(this);
    
    RenderManager::Instance()->ClearStats();
	Scene::Draw();
    RenderManager::Stats renderStats = RenderManager::Instance()->GetStats();
    SceneValidator::Instance()->CollectSceneStats(renderStats);
    
	DrawGrid();
}

#define GRIDMAX 500.0f
#define GRIDSTEP 10.0f
void EditorScene::DrawGrid()
{
    uint32 oldState = RenderManager::Instance()->GetState();	
	RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST); 
	RenderManager::Instance()->SetColor(0.7f, 0.7f, 0.7f, 1.0f);
	for (float32 x = -GRIDMAX; x <= GRIDMAX; x+=GRIDSTEP)
	{
		Vector3 v1(x, -GRIDMAX, 0);
		Vector3 v2(x, GRIDMAX, 0);
		
		Vector3 v3(-GRIDMAX, x, 0);
		Vector3 v4(GRIDMAX, x, 0);
		if (x!= 0.0f)
		{
			RenderHelper::Instance()->DrawLine(v1, v2);
			RenderHelper::Instance()->DrawLine(v3, v4);		
		}
	}
	RenderManager::Instance()->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	RenderHelper::Instance()->DrawLine(Vector3(-GRIDMAX, 0, 0), Vector3(GRIDMAX, 0, 0));
	RenderHelper::Instance()->DrawLine(Vector3(0, -GRIDMAX, 0), Vector3(0, GRIDMAX, 0));
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	RenderManager::Instance()->SetState(oldState);
}

void EditorScene::DrawDebugNodes(SceneNode * curr)
{
	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);	
	
	if (mesh && mesh->GetUserData())
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->GetUserData();
		data->bulletObject->Draw(mesh->GetWorldTransform(), mesh);
	}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		DrawDebugNodes(curr->GetChild(i));
	}
}


