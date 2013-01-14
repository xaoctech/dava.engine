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
	ParticleEffectSystem();
	virtual void Process();
};

}

#endif //__DAVAENGINE_SCENE3D_PARTICLEEFFECTSYSTEM_H__