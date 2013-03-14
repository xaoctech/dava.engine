/*
 *  DebugNode.h
 *  WoTSniperiPhone
 *
 *  Created by Yury Danilov on 29/11/11.
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#ifndef __DEBUG_NODE_H__
#define __DEBUG_NODE_H__

#include "DAVAEngine.h"

using namespace DAVA;

class DebugNode : public Entity
{
public:
	DebugNode(std::vector<float32> &data);
	virtual ~DebugNode();

	virtual void Draw();
	bool isDraw;
private:	
    RenderDataObject *renderData;
	std::vector<float32> verts;
	std::vector<float32> colors;
protected:

};

#endif
