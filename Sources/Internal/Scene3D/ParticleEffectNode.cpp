//
//  ParticleEffectNode.cpp
//  FrameworkQt
//
//  Created by Yuri Coder on 11/22/12.
//
//

#include "ParticleEffectNode.h"
#include "KeyedArchive.h"

using namespace DAVA;

#define PARTICLE_EFFECT_NODE_EMITTERS_COUNT "particleEffectNode.emittersCount"

ParticleEffectNode::ParticleEffectNode() : SceneNode()
{
    this->stopAfterNRepeats = -1;
    this->stopWhenEmpty = false;
    this->needEmitPlaybackComplete = false;
    this->playbackCompleteMessageSet = false;
}

void ParticleEffectNode::AddNode(SceneNode* node)
{
    SceneNode::AddNode(node);
    AddParticleEmitterNodeToMap(node);
}

void ParticleEffectNode::InsertBeforeNode(SceneNode *newNode, SceneNode *beforeNode)
{
    SceneNode::InsertBeforeNode(newNode, beforeNode);
    AddParticleEmitterNodeToMap(newNode);
}

void ParticleEffectNode::RemoveNode(SceneNode* node)
{
    SceneNode::RemoveNode(node);
    emitterPlaybackStatesMap.erase(node);
}

void ParticleEffectNode::AddParticleEmitterNodeToMap(SceneNode* node)
{
    ParticleEmitterNode* particleEmitterNode = dynamic_cast<ParticleEmitterNode*>(node);
    if (particleEmitterNode)
    {
        // Default Playback State is "Stopped".
        emitterPlaybackStatesMap.insert(std::make_pair(particleEmitterNode, false));
    }
}

void ParticleEffectNode::Start()
{
    emitterNodesCurrentlyPlaying.clear();

    for (EMITTERPLAYBACKSTATEMAPITER iter = this->emitterPlaybackStatesMap.begin();
         iter != this->emitterPlaybackStatesMap.end(); iter ++ )
    {
        // No need to do dynamic_cast here - we are sure that nodes in the map
        // are ParticleEmitterNodes only.
        ParticleEmitter* emitter = ((ParticleEmitterNode*)(iter->first))->GetEmitter();
        if (emitter)
        {
            emitter->Restart();

            emitterNodesCurrentlyPlaying.insert(iter->first);
            iter->second = true;
        }
    }

    needEmitPlaybackComplete = true;
}

void ParticleEffectNode::StopAfterNRepeats(int32 numberOfRepeats)
{
    this->stopAfterNRepeats = numberOfRepeats;
}

void ParticleEffectNode::StopWhenEmpty(bool value)
{
    this->stopWhenEmpty = value;
}

void ParticleEffectNode::Update(float32 timeElapsed)
{
    int32 childrenCount = GetChildrenCount();
    for (int i = 0; i < childrenCount; i ++)
    {
        SceneNode* childNode = GetChild(i);
        if (!childNode)
        {
            continue;
        }

        EMITTERPLAYBACKSTATEMAPITER iter = emitterPlaybackStatesMap.find(childNode);
        if (iter == this->emitterPlaybackStatesMap.end())
        {
            // Non-Particle Emitter Node, just update it.
            childNode->Update(timeElapsed);
            continue;
        }

        // This node is Particle Emitter, process it.
        if (iter->second == false)
        {
            // This emitter is already stopped - no need to update it.
            continue;
        }
 
        // No need to do dynamic_cast here - we are sure that nodes in the map
        // are ParticleEmitterNodes only.
        ParticleEmitter* emitter = ((ParticleEmitterNode*)(iter->first))->GetEmitter();
        emitter->Update(timeElapsed);
        if (IsStopEmitter(emitter))
        {
            emitterNodesCurrentlyPlaying.erase(iter->first);
            iter->second = false;
        }
    }

    // Are all the emitters finished playback? Notify user if yes.
    CheckPlaybackComplete();
}

void ParticleEffectNode::Draw()
{
    int32 childrenCount = GetChildrenCount();
    for (int i = 0; i < childrenCount; i ++)
    {
        SceneNode* childNode = GetChild(i);
        if (!childNode)
        {
            continue;
        }
        
        EMITTERPLAYBACKSTATEMAPITER iter = this->emitterPlaybackStatesMap.find(childNode);
        if (iter == this->emitterPlaybackStatesMap.end())
        {
            // Non-Particle Emitter Node, just draw it.
            childNode->Draw();
            continue;
        }

        // Draw only the items which are not in stopped state.
        if (iter->second == true)
        {
            iter->first->Draw();
        }
    }
}

bool ParticleEffectNode::IsStopEmitter(ParticleEmitter* emitter) const
{
    if (!emitter)
    {
        return true;
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

void ParticleEffectNode::CheckPlaybackComplete()
{
    if (this->needEmitPlaybackComplete == false)
    {
        // Already notified.
        return;
    }
    
    if (emitterNodesCurrentlyPlaying.size() == 0)
    {
        // Playback is finished!
        this->needEmitPlaybackComplete = false;
        
        if (this->playbackCompleteMessageSet)
        {
            this->playbackComplete(this, 0);
        }
    }
}

void ParticleEffectNode::SetPlaybackCompleteMessage(const Message& msg)
{
    this->playbackCompleteMessageSet = true;
    this->playbackComplete = msg;
}

void ParticleEffectNode::Save(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
    SceneNode::Save(archive, sceneFileV2);
    
    archive->SetInt32(PARTICLE_EFFECT_NODE_EMITTERS_COUNT, this->emitterPlaybackStatesMap.size());
    for (EMITTERPLAYBACKSTATEMAPITER iter = this->emitterPlaybackStatesMap.begin();
         iter != this->emitterPlaybackStatesMap.end(); iter ++ )
    {
        ParticleEmitterNode* emitterNode = ((ParticleEmitterNode*)(iter->first));
        emitterNode->Save(archive, sceneFileV2);
    }
}

void ParticleEffectNode::Load(KeyedArchive * archive, SceneFileV2 * sceneFileV2)
{
    SceneNode::Load(archive, sceneFileV2);

    int32 emittersCount = archive->GetInt32(PARTICLE_EFFECT_NODE_EMITTERS_COUNT, 0);
    for (int32 i = 0; i < emittersCount; i ++)
    {
        ParticleEmitterNode* childNode = new ParticleEmitterNode();
        childNode->Load(archive, sceneFileV2);
        
        SceneNode::AddNode(childNode);
        emitterPlaybackStatesMap.insert(std::make_pair(childNode, true));
    }
}
