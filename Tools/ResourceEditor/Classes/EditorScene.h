/*
 *  EditorScene.h
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

class EditorScene : public Scene
{
public:
	
    EditorScene();
    ~EditorScene();
    
    virtual void Update(float32 timeElapsed);

    btCollisionWorld *collisionWorld;
	void CheckNodes(SceneNode * curr);
	
	void TrySelection(Vector3 from, Vector3 direction);

	inline SceneNode * GetSelection()
	{
		return selection;
	}

	inline SceneNode * GetProxy()
	{
		return proxy;
	}
	
	
	void SetSelection(SceneNode *newSelection);
    
	virtual void Draw();
	void DrawGrid();
	void SetBulletUpdate(SceneNode* curr, bool value);
	void ReleaseUserData(SceneNode * curr);
	LandscapeNode * GetLandScape(SceneNode *node);
	
protected:

	SceneNode * GetHighestProxy(SceneNode* curr);

    btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btAxisSweep3* broadphase;
    int depth;
	
	SceneNode * selection;
	SceneNode * proxy;
	
	SceneNode * FindSelected(SceneNode * curr, btCollisionObject * coll);
	void DrawDebugNodes(SceneNode * curr);

	SceneNode * lastSelectedPhysics;
};

#endif