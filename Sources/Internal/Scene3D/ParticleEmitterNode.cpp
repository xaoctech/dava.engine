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

		const Matrix4 & mv = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
		Vector3 up(mv._01, mv._11, mv._21);
		Vector3 left(mv._00, mv._10, mv._20);

		emitter->Draw(up, left);

		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetBlendMode(sblend, dblend);
	}
}

};
