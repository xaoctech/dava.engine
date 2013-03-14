//
//  ParticleEffectNode.h
//  FrameworkQt
//
//  Created by Yuri Coder on 11/22/12.
//
//

#ifndef __DAVAENGINE_PARTICLE_EFFECT_NODE_H__
#define __DAVAENGINE_PARTICLE_EFFECT_NODE_H__

#include "Entity.h"
#include "ParticleEmitterNode.h"

namespace DAVA {

// Particle Effect Node.
class ParticleEffectNode : public Entity
{
public:
    ParticleEffectNode();

    /**
     \brief Access to Scene Nodes.
     */
	virtual void	AddNode(Entity * node);
    virtual void    InsertBeforeNode(Entity *newNode, Entity *beforeNode);

    /**
     \brief Start the playback for all inner nodes.
     */
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
     \brief Overloaded function of GameObject to update emitter when GameObjectManager updates GameObject.
     \param[in] timeElapsed time in seconds
	 */
	virtual void Update(float32 timeElapsed);

	virtual void Draw();

    /**
     \brief Set the message to be called when Playback is complete.
     */
    void SetPlaybackCompleteMessage(const Message& msg);

	virtual Entity* Clone(Entity *dstNode = NULL);

protected:
    // Check whether Node is Particle Emitter one and check the duration.
    bool PrepareNewParticleEmitterNode(Entity* node);
    
    // Update the duration for all the child nodes.
    void UpdateDurationForChildNodes(float32 newEmitterLifeTime);
    
    // Do we need to stop emitter?
    bool IsStopEmitter(ParticleEmitter* emitter) const;

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
};

}
#endif /* defined(#ifndef __DAVAENGINE_PARTICLE_EFFECT_NODE_H__) */
