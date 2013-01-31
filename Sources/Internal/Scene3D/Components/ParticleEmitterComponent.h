#ifndef __PARTICLE_EMITTER_COMPONENT_H__
#define __PARTICLE_EMITTER_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Base/BaseObject.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA
{

class ParticleEmitterComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(PARTICLE_EMITTER_COMPONENT);

	ParticleEmitterComponent();
	virtual ~ParticleEmitterComponent();

	void LoadFromYaml(const String& yamlPath);
	void SaveToYaml(const String& _yamlPath);
	String GetYamlPath();

	virtual Component * Clone(SceneNode * toEntity);

	void SetParticleEmitter(ParticleEmitter * particleEmitter);
	ParticleEmitter * GetParticleEmitter();

private:
	ParticleEmitter * particleEmitter;
    
public:
    INTROSPECTION_EXTEND(ParticleEmitterComponent, Component,
        MEMBER(particleEmitter, "Particle Emitter", INTROSPECTION_SERIALIZABLE | INTROSPECTION_EDITOR)
    );
};

}

#endif //__PARTICLE_EMITTER_COMPONENT_H__