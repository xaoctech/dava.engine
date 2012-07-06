#include "Scene3D/ParticleEmitterNode.h"
#include "Render/RenderManager.h"
#include "Scene3D/Scene.h"

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
		Matrix4 modelViewMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
		const Matrix4 & cameraMatrix = scene->GetCurrentCamera()->GetMatrix();
		Matrix4 meshFinalMatrix;
		////
		meshFinalMatrix = worldTransform * cameraMatrix;

		RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
		RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		RenderManager::Instance()->SetBlendMode(BLEND_ONE, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_WRITE);

		emitter->Draw(&meshFinalMatrix);

		RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
		RenderManager::Instance()->SetBlendMode(sblend, dblend);
		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, modelViewMatrix);
	}
}

};
