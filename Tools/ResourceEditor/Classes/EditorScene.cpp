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

/*
    This means that if we'll call GameScene->GetClassName() it'll return "Scene"
    This is for correct serialization of framework subclasses.
 */
REGISTER_CLASS_WITH_ALIAS(EditorScene, "Scene");

EditorScene::EditorScene()
{ 
	selection = 0;
//	dynCollisionConfiguration = new btDefaultCollisionConfiguration();
//	dynDispatcher = new	btCollisionDispatcher(dynCollisionConfiguration);
    btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
//	dynOverlappingPairCache = new btAxisSweep3(worldMin,worldMax);
//	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
//	dynSolver = sol;
//	dynamicsWorld = new btDiscreteDynamicsWorld(dynDispatcher, dynOverlappingPairCache, dynSolver
//                                                , dynCollisionConfiguration);
//	dynamicsWorld->setGravity(btVector3(0,0,-9.81));
        
    collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	collisionWorld = new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);
}

EditorScene::~EditorScene()
{

}

void EditorScene::Update(float32 timeElapsed)
{    
    Scene::Update(timeElapsed);
	CheckNodes(this);
	collisionWorld->updateAabbs();
}

void EditorScene::CheckNodes(SceneNode * curr)
{
	LightNode * light = dynamic_cast<LightNode *> (curr);
	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);	
	
	if (mesh && mesh->userData == 0)
	{
		SceneNodeUserData * data = new SceneNodeUserData();
		curr->userData = data;
		data->bulletObject = new BulletObject(this, collisionWorld, mesh, mesh->GetWorldTransform());
	}
	else if (mesh && mesh->userData)
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->userData;
		data->bulletObject->UpdateCollisionObject();
	}
	else if (light && light->userData == 0)
	{
		SceneNodeUserData * data = new SceneNodeUserData();
		curr->userData = data;
		data->bulletObject = new BulletObject(this, collisionWorld, light, light->GetWorldTransform());
		light->SetDebugFlags(DEBUG_DRAW_LIGHT_NODE);
	}
	else if (light && light->userData)
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->userData;
		data->bulletObject->UpdateCollisionObject();
	}
	
	

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		CheckNodes(curr->GetChild(i));
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
		if(selection)
		{
//			SceneNodeUserData * data = (SceneNodeUserData*)selection->userData;
//			if (data)
//			{
//				for (int i = cb.m_collisionObjects.size() - 1; i >= 0 ; i--)
//				{					
//					if (data->bulletObject->GetCollisionObject() == cb.m_collisionObjects[i])
//					{
//						findedIndex = i;
//						break;
//					}
//				}
//				while (findedIndex >= 0 && data->bulletObject->GetCollisionObject() == cb.m_collisionObjects[findedIndex])
//					findedIndex--;
//				findedIndex = findedIndex % cb.m_collisionObjects.size();
//			}
		}
		Logger::Debug("size:%d selIndex:%d", cb.m_collisionObjects.size(), findedIndex);
		
		if (findedIndex == -1)
			findedIndex = cb.m_collisionObjects.size() - 1;
		coll = cb.m_collisionObjects[findedIndex];
		selection = FindSelected(this, coll);
		
		if (selection)
		{
			SceneNode * solid = GetSolidParent(selection);
			if (solid)
				selection = solid;
		}
		if(selection)
			selection->SetDebugFlags(selection->GetDebugFlags() | (SceneNode::DEBUG_DRAW_AABOX_CORNERS));
	}
	else 
	{
		selection = 0;
	}
}

SceneNode * EditorScene::FindSelected(SceneNode * curr, btCollisionObject * coll)
{
	SceneNode * node = dynamic_cast<MeshInstanceNode *> (curr);
	if (node == 0)
		node = dynamic_cast<LightNode *> (curr);
	
	if (node && node->userData)
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->userData;
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

SceneNode * EditorScene::GetSelection()
{
	return selection;
}

void EditorScene::SetSelection(SceneNode *newSelection)
{
    if (selection)
    {
		selection->SetDebugFlags(selection->GetDebugFlags() & (~SceneNode::DEBUG_DRAW_AABOX_CORNERS));
    }
    
    selection = newSelection;
    
    if(selection)
    {
		selection->SetDebugFlags(selection->GetDebugFlags() | (SceneNode::DEBUG_DRAW_AABOX_CORNERS));
    }
}


void EditorScene::Draw()
{
	Scene::Draw();
//	DrawDebugNodes(this);
}

void EditorScene::DrawDebugNodes(SceneNode * curr)
{
	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);	
	
	if (mesh && mesh->userData)
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->userData;
		data->bulletObject->Draw(mesh->GetWorldTransform(), mesh);
	}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		DrawDebugNodes(curr->GetChild(i));
	}
}


