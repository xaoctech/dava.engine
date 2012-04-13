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
class HeightmapNode : public SceneNode
{
public:
    
    HeightmapNode(EditorScene * _scene);
    virtual ~HeightmapNode();

    const Vector3 &GetSize();

    float32 GetAreaScale();
    float32 GetSizeInMeters();
    
    virtual void	Draw();
    
    btCollisionObject *collisionObject;

protected:
    
    LandscapeNode *land;
    
    float32 areaScale;
    float32 maxHeight;
    
    btRigidBody* body;
    
    
    btHeightfieldTerrainShape* colShape;
    btDefaultMotionState *motionSate;
    
    RenderDataObject *renderData;

    Vector<uint16> hmap;
    
    Vector<float32> verts;
    Vector<float32> colors;
    Vector3 size;
    
    Vector3 position;
    float32 rotation;
    
    float32 sizeInMeters;
    
    EditorScene *editorScene;

};

#endif