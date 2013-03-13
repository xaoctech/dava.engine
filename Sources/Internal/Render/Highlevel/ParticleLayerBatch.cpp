#include "Render/Highlevel/ParticleLayerBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Material.h"
#include "Render/RenderManager.h"
#include "Particles/ParticleLayer.h"

namespace DAVA
{


ParticleLayerBatch::ParticleLayerBatch()
:	totalCount(0),
	particleLayer(0)
{

}

ParticleLayerBatch::~ParticleLayerBatch()
{
}

void ParticleLayerBatch::Draw(Camera * camera)
{
	if(!renderObject)return;
	//Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
	//if (!worldTransformPtr)return;

	uint32 flags = renderObject->GetFlags();
	if ((flags & RenderObject::VISIBILITY_CRITERIA) != RenderObject::VISIBILITY_CRITERIA)
		return;

	Matrix4 worldMatrix = Matrix4::IDENTITY;
	Matrix4 finalMatrix = worldMatrix * camera->GetMatrix();
	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

	particleLayer->Draw(camera);
	if(!totalCount)return;

	RenderManager::Instance()->SetRenderData(renderDataObject);
	material->PrepareRenderState();

	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, 6*totalCount);
}

void ParticleLayerBatch::SetTotalCount(int32 _totalCount)
{
	totalCount = _totalCount;
}

RenderBatch * ParticleLayerBatch::Clone()
{
	ParticleLayerBatch * rb = new ParticleLayerBatch();

	return rb;
}

void ParticleLayerBatch::SetParticleLayer(ParticleLayer * _particleLayer)
{
	particleLayer = _particleLayer;
}





}