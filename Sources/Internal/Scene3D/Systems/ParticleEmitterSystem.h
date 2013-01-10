#ifndef __DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__
#define __DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{

class ParticleEmitterSystem : public SceneSystem
{
public:
	virtual void AddEntity(SceneNode * entity);
	virtual void RemoveEntity(SceneNode * entity);
	virtual void Process();

private:
	Vector<SceneNode*> entities;
};

}

#endif //__DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__
