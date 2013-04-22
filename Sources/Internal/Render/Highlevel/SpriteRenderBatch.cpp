#include "Render/Highlevel/SpriteRenderBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/SpriteObject.h"
#include "Render/Highlevel/RenderFastNames.h"

namespace DAVA
{

SpriteRenderBatch::SpriteRenderBatch()
	: RenderBatch()
{
    SetOwnerLayerName(LAYER_TRANSLUCENT);
}

SpriteRenderBatch::~SpriteRenderBatch()
{
}

void SpriteRenderBatch::Draw(Camera * camera)
{
	if(!renderObject)return;
	Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
	if (!worldTransformPtr)
	{
		return;
	}

	uint32 flags = renderObject->GetFlags();
	if ((flags & RenderObject::VISIBILITY_CRITERIA) != RenderObject::VISIBILITY_CRITERIA)
		return;



	SpriteObject *spriteObject = static_cast<SpriteObject *>(renderObject);
	if(!spriteObject || !spriteObject->GetSprite())
	{
		return;
	}


	if(!RenderManager::Instance()->GetOptions()->IsOptionEnabled(RenderOptions::SPRITE_DRAW))
	{
		return;
	}

	const Matrix4 & cameraMatrix = camera->GetMatrix();
	Matrix4 finalMatrix;

	switch(spriteObject->GetSpriteType())
	{
	case SpriteObject::SPRITE_OBJECT:
		{
			finalMatrix = (*worldTransformPtr) * cameraMatrix;
			break;
		};
	case SpriteObject::SPRITE_BILLBOARD:
		{
			Matrix4 inverse(Matrix4::IDENTITY);

			inverse._00 = cameraMatrix._00;
			inverse._01 = cameraMatrix._10;
			inverse._02 = cameraMatrix._20;

			inverse._10 = cameraMatrix._01;
			inverse._11 = cameraMatrix._11;
			inverse._12 = cameraMatrix._21;

			inverse._20 = cameraMatrix._02;
			inverse._21 = cameraMatrix._12;
			inverse._22 = cameraMatrix._22;

			finalMatrix = inverse * (*worldTransformPtr) * cameraMatrix;
			break;
		};
	case SpriteObject::SPRITE_BILLBOARD_TO_CAMERA:
		{
 			Vector3 look = camera->GetPosition() - Vector3(0.0f, 0.0f, 0.0f) * (*worldTransformPtr); 
			look.Normalize();
			Vector3 right = CrossProduct(camera->GetUp(), look);
 			Vector3 up = CrossProduct(look, right);

			Matrix4 matrix = Matrix4::IDENTITY;
			matrix._00 = right.x;
			matrix._01 = right.y;
			matrix._02 = right.z;

			matrix._10 = up.x;
			matrix._11 = up.y;
			matrix._12 = up.z;

			matrix._20 = look.x;
			matrix._21 = look.y;
			matrix._22 = look.z;

			finalMatrix = matrix * (*worldTransformPtr) * cameraMatrix;
			break;
		};   
	}

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

	RenderManager::Instance()->SetRenderData(renderDataObject);
	materialInstance->PrepareRenderState();

	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, spriteObject->GetFrame() * 4, 4);
}


};

