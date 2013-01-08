#include "Scene3D/Systems/DeleteSystem.h"
#include "Scene3D/SceneNode.h"

namespace DAVA
{
	
DeleteSystem::DeleteSystem()
{
	deletedNodesToAdd = &deletedNodes1;
	deletedNodesToDelete = &deletedNodes2;
}

void DeleteSystem::Update()
{
	uint32 size = deletedNodesToDelete->size();
	for(uint32 i = 0; i < size; ++i)
	{
		SafeDelete(deletedNodesToDelete->at(i));
	}
	deletedNodesToDelete->clear();

	Vector<SceneNode*> * temp = deletedNodesToAdd;
	deletedNodesToAdd = deletedNodesToDelete;
	deletedNodesToDelete = temp;
}

void DeleteSystem::MarkNodeAsDeleted(SceneNode * node)
{
	deletedNodesToAdd->push_back(node);
}

DeleteSystem::~DeleteSystem()
{
	Update();
}

}
