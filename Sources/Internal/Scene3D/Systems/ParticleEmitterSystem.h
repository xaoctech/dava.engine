#ifndef __DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__
#define __DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/BaseProcessSystem.h"

namespace DAVA
{

class Component;
class ParticleEmitterSystem : public BaseProcessSystem
{
public:
	ParticleEmitterSystem();
	virtual void Process();
};

}

#endif //__DAVAENGINE_SCENE3D_PARTICLEEMITTERSYSTEM_H__
