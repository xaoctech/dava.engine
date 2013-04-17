/*
 *  HeightmapNode.h
 *  WoTSniperiPhone
 *
 *  Created by Alexey Prosin on 7/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __HEIGHTMAP_NODE_H__
#define __HEIGHTMAP_NODE_H__

#include "DAVAEngine.h"
#include "btBulletDynamicsCommon.h"

using namespace DAVA;
class btHeightfieldTerrainShape;
class EditorScene;
//class Heightmap;
class HeightmapNode : public Entity
{
public:
    
    HeightmapNode(EditorScene * _scene, Landscape *land);
    virtual ~HeightmapNode();

    const Vector3 &GetSize();

    float32 GetAreaScale();
    float32 GetSizeInMeters();
    
    btCollisionObject *collisionObject;

    void UpdateHeightmapRect(const Rect &rect);
    
protected:
    
    Landscape *land;
    
    float32 areaScale;
    float32 maxHeight;
    float32 heightScale;

    btRigidBody* body;
    
    
    btHeightfieldTerrainShape* colShape;
    btDefaultMotionState *motionSate;
    
    void SetValueToMap(int16 x, int16 y, float32 height, const AABBox3 &box);
    
    Vector<float32> hmap;
    
    Vector3 size;
    
    Vector3 position;
    float32 rotation;
    
    float32 sizeInMeters;
    
    Heightmap *heightmap;
    EditorScene *editorScene;
};

#endif