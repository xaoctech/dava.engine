#include "Scene3D/Systems/ParticleEffectSystem.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

ParticleEffectSystem::ParticleEffectSystem()
:	BaseProcessSystem(Component::PARTICLE_EFFECT_COMPONENT)
{

}

void ParticleEffectSystem::Process()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();

	uint32 size = components.size();
	for(uint32 i = 0; i < size; ++i)
	{
		ParticleEffectComponent * component = static_cast<ParticleEffectComponent*>(components[i]);
		component->EffectUpdate(timeElapsed);
	}
}

}