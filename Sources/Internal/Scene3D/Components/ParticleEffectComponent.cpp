#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitter.h"

namespace DAVA
{

ParticleEffectComponent::ParticleEffectComponent()
{
	stopAfterNRepeats = -1;
	stopWhenEmpty = false;
	effectDuration = 0.0f;
	emittersCurrentlyStopped = 0;
}

Component * ParticleEffectComponent::Clone(Entity * toEntity)
{
	ParticleEffectComponent * newComponent = new ParticleEffectComponent();
	newComponent->SetEntity(toEntity);

	newComponent->stopAfterNRepeats = stopAfterNRepeats;
	newComponent->stopWhenEmpty = stopWhenEmpty;
	newComponent->needEmitPlaybackComplete = needEmitPlaybackComplete;
	newComponent->playbackComplete = playbackComplete;
	newComponent->effectDuration = effectDuration;
	newComponent->emittersCurrentlyStopped = emittersCurrentlyStopped;

	return newComponent;
}

void ParticleEffectComponent::Start()
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->Play();
		}
	}

	this->emittersCurrentlyStopped = 0;
}

void ParticleEffectComponent::Stop()
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->Stop();
		}
		emittersCurrentlyStopped++;
	}
}

void ParticleEffectComponent::Restart()
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->Restart();
		}
	}
}

void ParticleEffectComponent::StopAfterNRepeats(int32 numberOfRepeats)
{
    stopAfterNRepeats = numberOfRepeats;
}

void ParticleEffectComponent::StopWhenEmpty(bool value /*= true*/)
{
	stopWhenEmpty = value;
}

void ParticleEffectComponent::EffectUpdate(float32 timeElapsed)
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			if (IsStopEmitter(emitter))
			{
				emitter->Stop();
				this->emittersCurrentlyStopped++;

				// Are all the emitters finished playback? Notify user if yes.
				CheckPlaybackComplete();
			}
		}
	}
}

void ParticleEffectComponent::SetPlaybackCompleteMessage(const Message & msg)
{
	playbackComplete = msg;
}

void ParticleEffectComponent::UpdateDurationForChildNodes(float32 newEmitterLifeTime)
{
	int32 childrenCount = entity->GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		RenderComponent * component = static_cast<RenderComponent*>(entity->GetChild(i)->GetComponent(Component::RENDER_COMPONENT));
		if(component && component->GetRenderObject() && component->GetRenderObject()->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(component->GetRenderObject());
			emitter->SetLifeTime(newEmitterLifeTime);
		}
	}
}

bool ParticleEffectComponent::IsStopEmitter(ParticleEmitter * emitter) const
{
	if (!emitter)
	{
		return true;
	}

	if (emitter->IsStopped())
	{
		// Already stopped - no need to stop again.
		return false;
	}

	// Check whether emitter is about to be stopped because of repeats count.
	if ((stopAfterNRepeats > 0) && (emitter->GetRepeatCount() >= stopAfterNRepeats))
	{
		return true;
	}

	// Check whether emitter is about to be stopped because of it is empty.
	if (stopWhenEmpty && emitter->GetParticleCount() == 0)
	{
		return true;
	}

	// No rules to stop emitter - continue its playback.
	return false;
}

void ParticleEffectComponent::CheckPlaybackComplete()
{
	if(entity->GetChildrenCount() == this->emittersCurrentlyStopped)
	{
		// Playback is finished!
		emittersCurrentlyStopped = 0;
		playbackComplete(entity, 0);
	}
}



}
