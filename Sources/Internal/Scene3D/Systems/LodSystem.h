#ifndef __DAVAENGINE_SCENE3D_LODSYSTEM_H__
#define __DAVAENGINE_SCENE3D_LODSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class Camera;

class LodSystem : public SceneSystem
{
public:
	LodSystem(Scene * scene);

	virtual void Process();
	virtual void AddEntity(SceneNode * entity);
	virtual void RemoveEntity(SceneNode * entity);

	virtual void SetCamera(Camera * camera);

	void UpdateEntityAfterLoad(SceneNode * entity);

private:
	//partial update per frame
	static const int32 UPDATE_PART_PER_FRAME = 1;
	Vector<int32> partialUpdateIndices;
	int32 currentPartialUpdateIndex;
	void UpdatePartialUpdateIndices();

	
	Vector<SceneNode*> entities;

	void UpdateLod(SceneNode * entity);
	void RecheckLod(SceneNode * entity);

	Camera * camera;
};

}

#endif //__DAVAENGINE_SCENE3D_LODSYSTEM_H__