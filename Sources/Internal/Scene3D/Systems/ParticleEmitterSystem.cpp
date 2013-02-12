#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

ParticleEmitterSystem::ParticleEmitterSystem(Scene * scene)
:	BaseProcessSystem(Component::PARTICLE_EMITTER_COMPONENT, scene)
{

}

void ParticleEmitterSystem::Process()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();

	uint32 size = components.size();
	for(uint32 i = 0; i < size; ++i)
	{
		ParticleEmitterComponent * component = static_cast<ParticleEmitterComponent*>(components[i]);
		component->GetParticleEmitter()->Update(timeElapsed);
	}
}



}
