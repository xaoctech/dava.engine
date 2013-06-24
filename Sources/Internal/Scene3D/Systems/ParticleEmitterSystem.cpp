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
				emitter->HandleRemoveFromSystem();
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
	Vector<ParticleEmitter*> emittersToBeDeleted;

	for(uint32 i = 0; i < size; ++i)
	{
		// Yuri Coder, 2013/05/15. Visible emitters are always updated, "deferred" update
		// is called for invisible ones. See pls issue #DF-1140.
		uint32 flags = emitters[i]->GetFlags();
        if ((flags & RenderObject::VISIBILITY_CRITERIA) == RenderObject::VISIBILITY_CRITERIA)
        {
            emitters[i]->Update(timeElapsed);
        }
		else
		{
			emitters[i]->DeferredUpdate(timeElapsed);
		}

		if (emitters[i]->IsToBeDeleted())
		{
			emittersToBeDeleted.push_back(emitters[i]);
		}
	}

	for(Vector<ParticleEmitter*>::iterator it = emittersToBeDeleted.begin(); it != emittersToBeDeleted.end(); ++it)
	{
		ParticleEmitter* partEmitter = (*it);
		RenderSystem* renderSystem = partEmitter->GetRenderSystem();
	
		renderSystem->RemoveFromRender(partEmitter);
		SafeRelease(partEmitter);
	}
}

}
