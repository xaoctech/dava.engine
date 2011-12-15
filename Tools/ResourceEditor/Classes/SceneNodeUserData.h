/*
 *  SceneNodeUserData.h
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#ifndef __SCENE_NODE_UD_H__
#define __SCENE_NODE_UD_H__

#include "DAVAEngine.h"
#include "btBulletDynamicsCommon.h"
#include "BulletObject.h"

using namespace DAVA;

class SceneNodeUserData : public BaseObject
{
public:
    
    SceneNodeUserData();
    ~SceneNodeUserData();
    
    BulletObject * bulletObject;
protected:
	
};

#endif