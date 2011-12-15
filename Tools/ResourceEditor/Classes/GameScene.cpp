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
	depth = 0;
	CheckNodes(this);
	Logger::Debug("CheckNodes end");
}

void GameScene::CheckNodes(SceneNode * curr)
{
	depth++;
	Logger::Debug("%d CheckNodes curr: %s", depth, curr->GetName().c_str());

	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);
	
	if (mesh && mesh->userData == 0)
	{
		SceneNodeUserData * data = new SceneNodeUserData();
		curr->userData = data;
		data->bulletObject = new BulletObject(this, collisionWorld, (MeshInstanceNode*)mesh, mesh->GetWorldTransform());
		mesh->AddNode(data->bulletObject->GetDebugNode());
	}
	else if (mesh && mesh->userData)
	{
		SceneNodeUserData * data = (SceneNodeUserData*)curr->userData;
		data->bulletObject->UpdateCollisionObject();
	}

	int size = curr->GetChildrenCount();
	Logger::Debug("GetChildrenCount %d",size);
	for (int i = 0; i < size; i++)
	{
		CheckNodes(curr->GetChild(i));
	}
	depth--;
}

void GameScene::Draw()
{
    Scene::Draw();
}

