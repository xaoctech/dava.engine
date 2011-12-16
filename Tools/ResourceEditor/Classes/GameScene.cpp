/*
 *  GameScene.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "GameScene.h"
#include "SceneNodeUserData.h"

GameScene::GameScene()
{ 
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

GameScene::~GameScene()
{

}

void GameScene::Update(float32 timeElapsed)
{    
    Scene::Update(timeElapsed);
//	depth = 0;
	CheckNodes(this);
//	Logger::Debug("CheckNodes end");
}

void GameScene::CheckNodes(SceneNode * curr)
{
//	depth++;
//	Logger::Debug("%d CheckNodes curr: %s", depth, curr->GetName().c_str());

	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);
	
	if (mesh && mesh->userData == 0)
	{
		SceneNodeUserData * data = new SceneNodeUserData();
		curr->userData = data;
		data->bulletObject = new BulletObject(this, collisionWorld, (MeshInstanceNode*)mesh, mesh->GetWorldTransform());
		mesh->AddNode(data->bulletObject->GetDebugNode());
		BulletLink link;
		link.bulletObj = data->bulletObject;
		link.sceneNode = curr;
		links.push_back(link);
//		Logger::Debug("%d Meshes count: %d", depth, mesh->GetMeshes().size());
	}
	else if (mesh && mesh->userData)
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->userData;
		data->bulletObject->UpdateCollisionObject();
	}

	int size = curr->GetChildrenCount();
//	Logger::Debug("GetChildrenCount %d",size);
	for (int i = 0; i < size; i++)
	{
		CheckNodes(curr->GetChild(i));
	}
}

void GameScene::TrySelection(Vector3 from, Vector3 direction)
{
	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(direction.x, direction.y, direction.z);
//	to = pos + to * 10000.0f;
	
	ShootTrace tr;
	tr.from = from;	
//	tr.to = from + direction * 10000.0f;
	tr.to = direction;
	traces.push_back(tr);
	
    btCollisionWorld::ClosestRayResultCallback cb(pos, to);
    collisionWorld->rayTest(pos, to, cb);
    if (cb.hasHit()) 
    {
		Logger::Debug("Has Hit");
		
		for (Vector<BulletLink>::iterator it = links.begin(); it != links.end(); it++) 
		{
			BulletLink & link = *it;
			link.bulletObj->GetDebugNode()->isDraw = (cb.m_collisionObject == link.bulletObj->GetCollisionObject());
		}		
    }
}

SceneNode * GameScene::GetSelection()
{
	return 0;
}


void GameScene::Draw()
{
    Scene::Draw();
    if (!traces.empty()) 
    {
        Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
        Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
        
        RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
        Color cr(1.0, 1.0, 1.0, 1.0);
        for (List<ShootTrace>::iterator it = traces.begin(); it != traces.end(); it++) 
        {
            cr.a = 1.0;
            RenderManager::Instance()->SetColor(cr);
            RenderHelper::Instance()->DrawLine(it->from, it->to);
        }
        RenderManager::Instance()->ResetColor();
        RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
    }
}

