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
	entities.push_back(entity);
}

void ParticleEmitterSystem::RemoveEntity(SceneNode * entity)
{
	uint32 size = entities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			return;
		}
	}

	DVASSERT(0);
}

void ParticleEmitterSystem::Process()
{
	float32 timeElapsed = SystemTimer::Instance()->FrameDelta();

	uint32 size = entities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		SceneNode * entity = entities[i];
		ParticleEmitterComponent * component = static_cast<ParticleEmitterComponent*>(entity->components[Component::PARTICLE_EMITTER_COMPONENT]);
		component->GetParticleEmitter()->Update(timeElapsed);
	}
}

}
