/*
 *  GameScene.h
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#ifndef __GAME_SCENE_H__
#define __GAME_SCENE_H__

#include "DAVAEngine.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "BulletObject.h"

using namespace DAVA;

struct BulletLink
{
	BulletObject * bulletObj;
	SceneNode * sceneNode;
};

class GameScene : public Scene
{
public:
    
	class ShootTrace
    {
    public:
        Vector3 from;
        Vector3 to;
    };
	
	
    GameScene();
    ~GameScene();
    
    virtual void Update(float32 timeElapsed);
    virtual void Draw();

//    void AddTank(HittableObject *hittableObject);
//    void AddBuilding(RealBuilding *building);
	

    btCollisionWorld *collisionWorld;
	void CheckNodes(SceneNode * curr);
	
	void TrySelection(Vector3 from, Vector3 direction);
	SceneNode * GetSelection();
	
	
protected:

    btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btAxisSweep3* broadphase;
    int depth;
	
    Vector<BulletLink> links;
	SceneNode * selection;
    List<ShootTrace> traces;
};

#endif