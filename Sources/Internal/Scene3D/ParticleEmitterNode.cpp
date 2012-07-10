#include "Scene3D/ParticleEmitterNode.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"
#include "Render/Material.h"

namespace DAVA
{

REGISTER_CLASS(ParticleEmitterNode);

ParticleEmitterNode::ParticleEmitterNode()
:	emitter(0)
{
}

ParticleEmitterNode::~ParticleEmitterNode()
{
	SafeRelease(emitter);
}

void ParticleEmitterNode::SetEmitter(ParticleEmitter3D * _emitter)
{
	SafeRelease(emitter);
	emitter = SafeRetain(_emitter);
}

void ParticleEmitterNode::Update(float32 timeElapsed)
{
	SceneNode::Update(timeElapsed);
	if(emitter)
	{
		Vector3 position = Vector3(worldTransform._30, worldTransform._31, worldTransform._32);
		emitter->SetPosition(position);
		emitter->Update(timeElapsed);
	}
}

void ParticleEmitterNode::Draw()
{
	if(emitter)
	{
		eBlendMode sblend = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dblend = RenderManager::Instance()->GetDestBlend();

		Camera * camera = scene->GetCurrentCamera();
		Vector3 up = camera->GetUp();
		Vector3 left = camera->GetLeft();
		Vector3 dir = camera->GetDirection();

		emitter->Draw(up, left, dir);

		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetBlendMode(sblend, dblend);
	}
}

};
