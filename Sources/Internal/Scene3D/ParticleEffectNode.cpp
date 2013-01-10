//
//  ParticleEffectNode.cpp
//  FrameworkQt
//
//  Created by Yuri Coder on 11/22/12.
//
//

#include "ParticleEffectNode.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/RenderManager.h"
#include "Render/RenderHelper.h"

using namespace DAVA;
REGISTER_CLASS(ParticleEffectNode);

ParticleEffectNode::ParticleEffectNode() : SceneNode()
{
    this->stopAfterNRepeats = -1;
    this->stopWhenEmpty = false;
    this->effectDuration = 0.0f;
    this->emittersCurrentlyStopped = 0;
    
    SetName("Particle effect");
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
    //emitter->Stop();
    
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

void ParticleEffectNode::Stop()
{
	int32 childrenCount = GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		ParticleEmitterNode* particleEmitterNode = static_cast<ParticleEmitterNode*>(GetChild(i));
		particleEmitterNode->GetEmitter()->Stop();
		emittersCurrentlyStopped++;
	}
}

void ParticleEffectNode::Restart()
{
	int32 childrenCount = GetChildrenCount();
	for (int32 i = 0; i < childrenCount; i ++)
	{
		ParticleEmitterNode* particleEmitterNode = static_cast<ParticleEmitterNode*>(GetChild(i));
		particleEmitterNode->GetEmitter()->Pause(false);
		particleEmitterNode->GetEmitter()->Restart(true);
	}
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

void DAVA::ParticleEffectNode::Draw()
{
	SceneNode::Draw();

	if (debugFlags != DEBUG_DRAW_NONE)
	{
		if (!(flags & SceneNode::NODE_VISIBLE))return;

		RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
		RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE); 

		Vector3 position = Vector3(0.0f, 0.0f, 0.0f) * GetWorldTransform();
		Matrix3 rotationPart(GetWorldTransform());
		Vector3 direction = Vector3(0.0f, 0.0f, 1.0f) * rotationPart;
		direction.Normalize();

		RenderManager::Instance()->SetColor(0.0f, 0.0f, 1.0f, 1.0f); 

		RenderHelper::Instance()->DrawLine(position, position + direction * 10, 2.f);        

		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
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
        this->playbackComplete(this, 0);
    }
}

void ParticleEffectNode::SetPlaybackCompleteMessage(const Message& msg)
{
    this->playbackComplete = msg;
}

SceneNode* ParticleEffectNode::Clone(SceneNode *dstNode /*= NULL*/)
{
	if (!dstNode) 
	{
		DVASSERT_MSG(IsPointerToExactClass<ParticleEffectNode>(this), "Can clone only ParticleEffectNode");
		dstNode = new ParticleEffectNode();
	}

	SceneNode::Clone(dstNode);
	ParticleEffectNode *nd = (ParticleEffectNode *)dstNode;
	nd->effectDuration = effectDuration;

	return dstNode;
}

