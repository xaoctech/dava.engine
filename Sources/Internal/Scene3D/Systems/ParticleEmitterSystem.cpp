#include "Scene3D/Systems/ParticleEmitterSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/SystemTimer.h"
#include "Base/TemplateHelpers.h"

namespace DAVA
{


void ParticleEmitterSystem::AddIfEmitter(RenderObject * maybeEmitter)
{
	if(maybeEmitter->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(maybeEmitter);
		emitters.push_back(emitter);
	}
}

void ParticleEmitterSystem::RemoveIfEmitter(RenderObject * maybeEmitter)
{
	if(maybeEmitter->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
	{
		ParticleEmitter * emitter = static_cast<ParticleEmitter*>(maybeEmitter);
		uint32 size = emitters.size();
		for(uint32 i = 0; i < size; ++i)
		{
			if(emitters[i] == emitter)
			{
				emitters[i] = emitters[size-1];
				emitters.pop_back();
				return;
			}
		}
		DVASSERT(0);
	}
}

void ParticleEmitterSystem::Update(float32 timeElapsed)
{
	uint32 size = emitters.size();
	for(uint32 i = 0; i < size; ++i)
	{
		
		// Yuri Coder, 2013/05/15. Have to update all emitters, even non-visible ones because
		// of DF-1140 issue.
        emitters[i]->Update(timeElapsed);
	}
}

}
