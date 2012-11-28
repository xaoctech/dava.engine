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
    this->playbackCompleteMessageSet = false;
    this->effectDuration = 0.0f;
    this->emittersCurrentlyStopped = 0;
}

void ParticleEffectNode::AddNode(SceneNode* node)
{
    if (PrepareNewParticleEmitterNode(node))
    {
        SceneNode::AddNode(node);
    }
}

void ParticleEffectNode::InsertBeforeNode(SceneNode *newNode, SceneNode *beforeNode)
{
    if (PrepareNewParticleEmitterNode(newNode))
    {
        SceneNode::InsertBeforeNode(newNode, beforeNode);
    }
}

bool ParticleEffectNode::PrepareNewParticleEmitterNode(SceneNode* node)
{
    // Only Particle Emitter nodes are allowed.
    ParticleEmitterNode* particleEmitterNode = dynamic_cast<ParticleEmitterNode*>(node);
    if (!particleEmitterNode)
    {
        Logger::Warning("ParticleEffectNode::PrepareNewParticleEmitterNode() - attempt to add child node with wrong type!");
        return false;
    }
    
    ParticleEmitter* emitter = particleEmitterNode->GetEmitter();
    if (!emitter)
    {
        Logger::Error("ParticleEffectNode::PrepareNewParticleEmitterNode() - no Emitter exists!");
        return false;
    }

    // Default Node State is Stopped.
    emitter->Stop();
    
    // The effect duration is the same as longest lifetime of the child nodes.
    float32 newEmitterLifeTime = emitter->GetLifeTime();
    if (newEmitterLifeTime > effectDuration)
    {
        this->effectDuration = newEmitterLifeTime;
        UpdateDurationForChildNodes(newEmitterLifeTime);
    }
    else
    {
        emitter->SetLifeTime(effectDuration);
    }

    return true;
}

void ParticleEffectNode::UpdateDurationForChildNodes(float32 newEmitterLifeTime)
{
    int32 childrenCount = GetChildrenCount();
    for (int32 i = 0; i < childrenCount; i ++)
    {
        ParticleEmitterNode* particleEmitterNode = dynamic_cast<ParticleEmitterNode*>(GetChild(i));
        if (particleEmitterNode)
        {
            particleEmitterNode->GetEmitter()->SetLifeTime(newEmitterLifeTime);
        }
    }
}

void ParticleEffectNode::Start()
{
    int32 childrenCount = GetChildrenCount();
    for (int32 i = 0; i < childrenCount; i ++)
    {
        ParticleEmitterNode* particleEmitterNode = static_cast<ParticleEmitterNode*>(GetChild(i));
        particleEmitterNode->GetEmitter()->Play();
    }

    this->emittersCurrentlyStopped = 0;
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
    SceneNode::Update(timeElapsed);

    int32 childrenCount = GetChildrenCount();
    for (int32 i = 0; i < childrenCount; i ++)
    {
        ParticleEmitter* emitter = static_cast<ParticleEmitterNode*>(GetChild(i))->GetEmitter();
        if (IsStopEmitter(emitter))
        {
            emitter->Stop();
            this->emittersCurrentlyStopped ++;

            // Are all the emitters finished playback? Notify user if yes.
            CheckPlaybackComplete();
        }
    }
}

bool ParticleEffectNode::IsStopEmitter(ParticleEmitter* emitter) const
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

void ParticleEffectNode::CheckPlaybackComplete()
{
    if (GetChildrenCount() == this->emittersCurrentlyStopped)
    {
        // Playback is finished!
        this->emittersCurrentlyStopped = 0;
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

    int32 childrenCount = this->GetChildrenCount();
    archive->SetInt32(PARTICLE_EFFECT_NODE_EMITTERS_COUNT, childrenCount);
    for (int32 i = 0; i < childrenCount; i ++)
    {
        ParticleEmitterNode* emitterNode = static_cast<ParticleEmitterNode*>(GetChild(i));
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
    }
}
