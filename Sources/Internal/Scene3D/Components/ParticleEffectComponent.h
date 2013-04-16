#ifndef __PARTICLE_EFFECT_COMPONENT_H__
#define __PARTICLE_EFFECT_COMPONENT_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"
#include "Base/BaseObject.h"
#include "Base/Message.h"

namespace DAVA
{

class ParticleEmitter;
class ParticleEffectComponent : public Component
{
public:
	IMPLEMENT_COMPONENT_TYPE(PARTICLE_EFFECT_COMPONENT);

	ParticleEffectComponent();

	virtual Component * Clone(Entity * toEntity);

	void Start();

	void Stop();

	void Restart();
    
    /**
     \brief Function marks that all the emitters must be stopped after N repeats of emitter animation.
     \param[in] numberOfRepeats number of times we need to repeat emitter animation before stop.
     */
    void StopAfterNRepeats(int32 numberOfRepeats);

    /**
     \brief Function marks that this object must be stopped when number of particles will be equal to 0
     */
    void StopWhenEmpty(bool value = true);

    /**
     \brief Per-frame update
     \param[in] timeElapsed time in seconds
	 */
	virtual void EffectUpdate(float32 timeElapsed);

    /**
     \brief Set the message to be called when Playback is complete.
     */
    void SetPlaybackCompleteMessage(const Message & msg);

	/**
     \brief Access to playback speed for the particle emitters. Returns
	 the playback speed for first emitter, sets for all ones.
     */
	float32 GetPlaybackSpeed();
	void SetPlaybackSpeed(float32 value);

	/**
     \brief Returns the total active particles count for the whole effect.
     */
	int32 GetActiveParticlesCount();

protected:
	// Update the duration for all the child nodes.
	void UpdateDurationForChildNodes(float32 newEmitterLifeTime);

	// Do we need to stop emitter?
	bool IsStopEmitter(ParticleEmitter * emitter) const;

	// Check the "Playback Complete", emit a message, if needed.
	void CheckPlaybackComplete();

private:
	// "Stop after N repeats" value.
	int32 stopAfterNRepeats;

	// "Stop if the emitter is empty" value.
	bool stopWhenEmpty;

	// Whether we need to emit "Playback Complete" event?
	bool needEmitPlaybackComplete;

	// Playback complete message.
	Message playbackComplete;

	// Effect duration - common for all emitters.
	float32 effectDuration;

	// Count of emitters currently stopped.
	int32 emittersCurrentlyStopped;

public:
	INTROSPECTION_EXTEND(ParticleEffectComponent, Component,
		MEMBER(stopAfterNRepeats, "stopAfterNRepeats", INTROSPECTION_SERIALIZABLE)
        MEMBER(stopWhenEmpty, "stopWhenEmpty", INTROSPECTION_SERIALIZABLE)
//        MEMBER(needEmitPlaybackComplete, "needEmitPlaybackComplete", INTROSPECTION_SERIALIZABLE)
        MEMBER(effectDuration, "effectDuration", INTROSPECTION_SERIALIZABLE)
    );
};

}

#endif //__PARTICLE_EFFECT_COMPONENT_H__