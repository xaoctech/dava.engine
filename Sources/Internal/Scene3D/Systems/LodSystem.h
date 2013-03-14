#ifndef __DAVAENGINE_SCENE3D_LODSYSTEM_H__
#define __DAVAENGINE_SCENE3D_LODSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class Camera;
class LodComponent;

class LodSystem : public SceneSystem
{
public:
	LodSystem(Scene * scene);

	virtual void Process();
	virtual void AddEntity(Entity * entity);
	virtual void RemoveEntity(Entity * entity);

	virtual void SetCamera(Camera * camera);

	static void UpdateEntityAfterLoad(Entity * entity);

	static void MergeChildLods(Entity * toEntity);

	class LodMerger
	{
	public:
		LodMerger(Entity * toEntity);
		void MergeChildLods();

	private:
		void GetLodComponentsRecursive(Entity * fromEntity, Vector<Entity*> & allLods);
		Entity * toEntity;
	};

	

private:
	//partial update per frame
	static const int32 UPDATE_PART_PER_FRAME = 1;
	Vector<int32> partialUpdateIndices;
	int32 currentPartialUpdateIndex;
	void UpdatePartialUpdateIndices();

	
	Vector<Entity*> entities;

	void UpdateLod(Entity * entity);
	void RecheckLod(Entity * entity);

	Camera * camera;
};

}

#endif //__DAVAENGINE_SCENE3D_LODSYSTEM_H__