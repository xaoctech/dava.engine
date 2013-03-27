#ifndef __DAVAENGINE_SCENE3D_PARTICLEEFFECTSYSTEM_H__
#define __DAVAENGINE_SCENE3D_PARTICLEEFFECTSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/BaseProcessSystem.h"

namespace DAVA
{

class Component;
class ParticleEffectSystem : public BaseProcessSystem
{
public:
	ParticleEffectSystem(Scene * scene);
	virtual void Process();

	virtual void RemoveEntity(Entity * entity);

	uint32 index;
	uint32 size;
};

}

#endif //__DAVAENGINE_SCENE3D_PARTICLEEFFECTSYSTEM_H__