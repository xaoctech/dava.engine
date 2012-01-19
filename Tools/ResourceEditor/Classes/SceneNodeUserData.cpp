/*
 *  SceneNodeUserData.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "SceneNodeUserData.h"

SceneNodeUserData::SceneNodeUserData()
{ 
	bulletObject = 0;
}

SceneNodeUserData::~SceneNodeUserData()
{
	SafeRelease(bulletObject);
}

