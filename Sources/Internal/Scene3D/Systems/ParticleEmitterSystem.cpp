#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Scene3D/Components/ParticleEmitterComponent.h"
#include "Scene3D/SceneNode.h"
#include "Particles/ParticleEmitter.h"
#include "Debug/DVAssert.h"
#include "Platform/SystemTimer.h"

namespace DAVA
{

void ParticleEmitterSystem::AddEntity(SceneNode * entity)
{
	components.push_back(entity->components[Component::PARTICLE_EMITTER_COMPONENT]);
}

void ParticleEmitterSystem::RemoveEntity(SceneNode * entity)
{
	uint32 size = components.size();
	Component * deletingComponent = entity->components[Component::PARTICLE_EMITTER_COMPONENT];
	for(uint32 i = 0; i < size; ++i)
	{
		if(components[i] == deletingComponent)
		{
			components[i] = components[size-1];
			components.pop_back();
			return;
		}
	}

	DVASSERT(0);
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
