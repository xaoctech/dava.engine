//
//  ParticleEffectNode.h
//  FrameworkQt
//
//  Created by Yuri Coder on 11/22/12.
//
//

#ifndef __FrameworkQt__ParticleEffectNode__
#define __FrameworkQt__ParticleEffectNode__

#include "SceneNode.h"
#include "ParticleEmitterNode.h"

namespace DAVA {

// Particle Effect Node.
class ParticleEffectNode : public SceneNode
{
public:
    ParticleEffectNode();

    /**
     \brief Access to Scene Nodes.
     */
	virtual void	AddNode(SceneNode * node);
    virtual void    InsertBeforeNode(SceneNode *newNode, SceneNode *beforeNode);
	virtual void	RemoveNode(SceneNode * node);
    
    /**
     \brief Save Particle Effect Node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2);
    
    /**
     \brief Load Particle Effect Node from KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2);

    /**
     \brief Start the playback for all inner nodes.
     */
    void Start();
    
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

	/**
     \brief Overloaded function of GameObject to draw emitter when GameObjectManager draws GameObject.
     */
	virtual void Draw();

    /**
     \brief Set the message to be called when Playback is complete.
     */
    void SetPlaybackCompleteMessage(const Message& msg);

protected:
    // Check whether Node is Particle Emitter one, add to map if yes.
    void AddParticleEmitterNodeToMap(SceneNode* node);
    
    // Do we need to stop emitter?
    bool IsStopEmitter(ParticleEmitter* emitter) const;

    // Check the "Playback Complete", emit a message, if needed.
    void CheckPlaybackComplete();
    
private:
    // States of Particle Emitter Nodes - whether they are started or not.
    typedef Map<SceneNode*, bool> EMITTERPLAYBACKSTATEMAP;
    typedef EMITTERPLAYBACKSTATEMAP::iterator EMITTERPLAYBACKSTATEMAPITER;

    EMITTERPLAYBACKSTATEMAP emitterPlaybackStatesMap;
    
    // Set of Particle Emitter nodes currently playing.
    Set<SceneNode*> emitterNodesCurrentlyPlaying;
    
    // "Stop after N repeats" value.
    int32 stopAfterNRepeats;
    
    // "Stop if the emitter is empty" value.
    bool stopWhenEmpty;
    
    // Whether we need to emit "Playback Complete" event?
    bool needEmitPlaybackComplete;
    
    // Playback complete message.
    bool playbackCompleteMessageSet;
    Message playbackComplete;
};

}
#endif /* defined(__FrameworkQt__ParticleEffectNode__) */
