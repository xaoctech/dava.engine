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

static const FastName LAYER_ARROWS("ArrowsRenderLayer");

class HeightmapNode;
class EditorScene : public Scene
{
public:
	
    EditorScene();
    ~EditorScene();
    
    virtual void Update(float32 timeElapsed);
	virtual void RemoveNode(Entity * node);
	void UpdateBullet(Entity * curr);

    btCollisionWorld *collisionWorld;
	btCollisionWorld *landCollisionWorld;
	void CheckNodes(Entity * curr);
	
	void TrySelection(Vector3 from, Vector3 direction);
	bool TryIsTargetAccesible(Vector3 from, Vector3 target);
	void JuncCollWorldToLandscapeCollWorld();
	void SeparateCollWorldFromLandscapeCollWorld();
    bool LandscapeIntersection(const Vector3 &from, const Vector3 &direction, Vector3 &point); 

	inline Entity * GetSelection()
	{
		return selection;
	}

	inline Entity * GetProxy()
	{
		return proxy;
	}
	
	
	void SetSelection(Entity *newSelection);
    
	virtual void Draw();
	void DrawGrid();
	void SetBulletUpdate(Entity* curr, bool value);
	void ReleaseUserData(Entity * curr);
	static Landscape * GetLandscape(Entity *node);
	static Entity* GetLandscapeNode(Entity *node);
    
    void SetDrawGrid(bool newDrawGrid);
	
    void SetForceLodLayer(Entity *node, int32 layer);
    int32 GetForceLodLayer(Entity *node);
    
    const RenderManager::Stats & GetRenderStats() const;
    
protected:
    void SetForceLodLayerRecursive(Entity *node, int32 layer);

    btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btAxisSweep3* broadphase;
    int depth;

	btDefaultCollisionConfiguration* landCollisionConfiguration;
	btCollisionDispatcher* landDispatcher;
	btAxisSweep3* landBroadphase;

	Entity * selection;
	Entity * proxy;
    
    Entity *selectedEntity;
	
	Entity * FindSelected(Entity * curr, btCollisionObject * coll);
	HeightmapNode * FindHeightmap(Entity * curr, btCollisionObject * coll);

	Entity * lastSelectedPhysics;
    bool drawGrid;

	btBroadphaseProxy* originalHandler;
    
    RenderManager::Stats renderStats;
};

#endif