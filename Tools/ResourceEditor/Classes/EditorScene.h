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

class HeightmapNode;
class EditorScene : public Scene
{
public:
	
    EditorScene();
    ~EditorScene();
    
    virtual void Update(float32 timeElapsed);
	void UpdateBullet(SceneNode * curr);

    btCollisionWorld *collisionWorld;
	btCollisionWorld *landCollisionWorld;
	void CheckNodes(SceneNode * curr);
	
	void TrySelection(Vector3 from, Vector3 direction);
	bool TryIsTargetAccesible(Vector3 from, Vector3 target);
	void JuncCollWorldToLandscapeCollWorld();
	void SeparateCollWorldFromLandscapeCollWorld();
    bool LandscapeIntersection(const Vector3 &from, const Vector3 &direction, Vector3 &point); 

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
	static Landscape * GetLandscape(SceneNode *node);
	static SceneNode* GetLandscapeNode(SceneNode *node);
    
    void SetDrawGrid(bool newDrawGrid);
	
    void SetForceLodLayer(SceneNode *node, int32 layer);
    int32 GetForceLodLayer(SceneNode *node);
    
protected:

    
    void SetNodeDebugFlags(SceneNode *selectedNode, uint32 flags);
    
    void SetForceLodLayerRecursive(SceneNode *node, int32 layer);

    btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btAxisSweep3* broadphase;
    int depth;

	btDefaultCollisionConfiguration* landCollisionConfiguration;
	btCollisionDispatcher* landDispatcher;
	btAxisSweep3* landBroadphase;

	SceneNode * selection;
	SceneNode * proxy;
    
    SceneNode *selectedEntity;
	
	SceneNode * FindSelected(SceneNode * curr, btCollisionObject * coll);
	HeightmapNode * FindHeightmap(SceneNode * curr, btCollisionObject * coll);

	SceneNode * lastSelectedPhysics;
    
    bool drawGrid;

	btBroadphaseProxy* originalHandler;
};

#endif