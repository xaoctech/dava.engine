/*
 *  GameScene.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "GameScene.h"

GameScene::GameScene()
{ 
    dynCollisionConfiguration = new btDefaultCollisionConfiguration();
	dynDispatcher = new	btCollisionDispatcher(dynCollisionConfiguration);
    btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	dynOverlappingPairCache = new btAxisSweep3(worldMin,worldMax);
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	dynSolver = sol;
	dynamicsWorld = new btDiscreteDynamicsWorld(dynDispatcher, dynOverlappingPairCache, dynSolver
                                                , dynCollisionConfiguration);
	
	dynamicsWorld->setGravity(btVector3(0,0,-9.81));
	
	btStaticPlaneShape * colShape = new btStaticPlaneShape(btVector3(0.0f, 0.0f, 1.0f), btScalar(0.0f));    
		    
	/// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    
    btScalar	mass(0.0f);
	//rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    btVector3 localInertia(0,0,0);
    if (isDynamic)
        colShape->calculateLocalInertia(mass,localInertia);
        
    startTransform.setOrigin(btVector3( btScalar(0),
                                       btScalar(0),
                                       btScalar(0)));
    
    startTransform.setRotation(btQuaternion(btVector3(0,0,1), 0.0f));
    
	//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
//    btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
//    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
//    rbInfo.m_friction = 0.5;
//    btRigidBody * body = new btRigidBody(rbInfo);
//    body->setContactProcessingThreshold(BT_LARGE_FLOAT);
//    dynamicsWorld->addRigidBody(body);
    
}

GameScene::~GameScene()
{

}


//void GameScene::AddTank(HittableObject *hittableObject)
//{
//    hittableObjects.push_back(hittableObject);
//    hittableObject->Retain();
//	AddNode((SceneNode*)hittableObject);
//}
//
//void GameScene::AddBuilding(RealBuilding *building)
//{
//    buildingObjects.push_back(building);
//    building->Retain();
//	AddNode((SceneNode*)building);	
//}



//void GameScene::HitOnEnemy(btCollisionWorld::ClosestRayResultCallback *rayCallback, float32 damage)
//{
//    for (Vector<HittableObject*>::iterator it = hittableObjects.begin(); it != hittableObjects.end(); it++) 
//    {
//        int32 hitPart = (*it)->IsRayCollided(rayCallback);
//        if (hitPart != -1) 
//        {
//            (*it)->Hit(rayCallback, hitPart, damage);
//        }
//    }
//}


void GameScene::Update(float32 timeElapsed)
{    
    dynamicsWorld->stepSimulation(timeElapsed, 1000);
    Scene::Update(timeElapsed);
    
//    for (List<ShootObject>::iterator it = shoots.begin(); it != shoots.end(); it++) 
//    {
//        if (it->flightTime < 1.f) 
//        {
//            ShootTrace tr;
//            tr.intency = 1.0f;
//            tr.from = Vector3(it->from.getX(), it->from.getY(), it->from.getZ());
//            btVector3 to = it->from + it->direction * timeElapsed;
//            if (it->damage == 0)
//            {
//                to = it->from + it->direction;
//            }
//                
//            tr.to = Vector3(to.getX(), to.getY(), to.getZ());
//            if (it->damage > 0)
//            {
//                traces.push_back(tr);
//            }
//
//            SelectiveRayResultCallback cb(it->owner, it->from, to);
//            dynamicsWorld->rayTest(it->from, to, cb);
//            
//            if ( it->damage == 0 )//just aiming
//            {
//				if (cb.hasHit())
//				{
//					float32 aimLen = sqrt((cb.m_hitPointWorld.getX() - it->from.getX()) * (cb.m_hitPointWorld.getX() - it->from.getX()) + (cb.m_hitPointWorld.getY() - it->from.getY()) * (cb.m_hitPointWorld.getY() - it->from.getY()));
//					playerTank->GetGunAim()->SetDistFromTank(aimLen);
//					it->flightTime = 2.f;
//					continue;
//				}
//				else
//				{
//					playerTank->GetGunAim()->SetDistFromTank(20);
//				}
//            }
//			
//            if (cb.hasHit()) 
//            {
//                Logger::Info("!!!   HIT SOMETHING   !!!");
//                AddExplosion(Vector3(cb.m_hitPointWorld.getX()
//                                     , cb.m_hitPointWorld.getY()
//                                     , cb.m_hitPointWorld.getZ()));
//                if (it->owner == playerTank.Get()) 
//                {
//                    HitOnEnemy(&cb, it->damage);
//                }
//                else 
//                {
//                    int32 hitPart = playerTank->IsRayCollided(&cb);
//                    if (hitPart != TankObject::COLLISION_NONE) 
//                    {
//                        playerTank->Hit(&cb, hitPart, it->damage);
//                    }
//                    else 
//                    {
//                        HitOnEnemy(&cb, it->damage);
//                    }
//                }
//                it->flightTime = 2.f;
//            }
//            else
//            {
//                it->flightTime += timeElapsed;
//                it->from = to;
//            }
//        }
//    }
}

void GameScene::Draw()
{
    Scene::Draw();
}

