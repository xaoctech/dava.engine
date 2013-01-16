#ifndef __PARTICLE_EMITTER_COMPONENT_H__
#define __PARTICLE_EMITTER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Base/BaseObject.h"

namespace DAVA
{

class ParticleEmitter;
class ParticleEmitterComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(PARTICLE_EMITTER_COMPONENT);

	ParticleEmitterComponent();
	virtual ~ParticleEmitterComponent();

	virtual Component * Clone();

	void SetParticleEmitter(ParticleEmitter * particleEmitter);
	ParticleEmitter * GetParticleEmitter();

private:
	ParticleEmitter * particleEmitter;
    
public:
    INTROSPECTION_EXTEND(ParticleEmitterComponent, Component,
		NULL);
};

}

#endif //__PARTICLE_EMITTER_COMPONENT_H__