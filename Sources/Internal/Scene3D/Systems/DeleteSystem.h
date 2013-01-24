#ifndef __DAVAENGINE_DELETE_SYSTEM_H__
#define __DAVAENGINE_DELETE_SYSTEM_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{

class SceneNode;

class DeleteSystem
{
public:
	DeleteSystem();
	virtual ~DeleteSystem();

	void Process();

	void MarkNodeAsDeleted(SceneNode * node);

private:
	Vector<SceneNode*> deletedNodes1;
	Vector<SceneNode*> deletedNodes2;

	Vector<SceneNode*> * deletedNodesToAdd;
	Vector<SceneNode*> * deletedNodesToDelete;
};

}; 

#endif //__DAVAENGINE_DELETE_SYSTEM_H__