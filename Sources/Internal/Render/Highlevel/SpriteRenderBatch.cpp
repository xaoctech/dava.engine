#include "Render/Highlevel/SpriteRenderBatch.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/SpriteObject.h"


namespace DAVA
{

SpriteRenderBatch::SpriteRenderBatch()
	: RenderBatch()
{
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

	// Get current modelview matrix, and in this case it's always a camera matrix
	Matrix4 modelViewMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
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

// 			finalMatrix = inverse * (*worldTransformPtr) * modelViewMatrix;
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

//  			finalMatrix = matrix * (*worldTransformPtr) * modelViewMatrix;
			finalMatrix = matrix * (*worldTransformPtr) * cameraMatrix;

			//            left.x = cameraTransform._00;
			//            left.y = cameraTransform._10;
			//            left.z = cameraTransform._20;
			//            up.x = cameraTransform._01;
			//            up.y = cameraTransform._11;
			//            up.z = cameraTransform._21;
			//            target.x = position.x - cameraTransform._02;
			//            target.y = position.y - cameraTransform._12;
			//            target.z = position.z - cameraTransform._22;
			break;
		};   
	}


	//TODO: Add billboards mode
	//    {//billboarding
	//        Vector3 camDir = scene->GetCamera()->GetDirection();
	//        Vector3 right;
	//        Vector3 up(0.f,0.f,1.f);
	//        right = up.CrossProduct(camDir);
	//        up = camDir.CrossProduct(right);
	////        up.Normalize();
	////        right.Normalize();
	//        
	//        Matrix4 bm;
	//        
	//        bm._00 = right.x;
	//        bm._10 = right.y;
	//        bm._20 = right.z;
	//        bm._30 = 0.0f;
	//        
	//        bm._01 = up.x;
	//        bm._11 = up.y;
	//        bm._21 = up.z;
	//        bm._31 = 0.0f;
	//        
	//        bm._02 = camDir.x;
	//        bm._12 = camDir.y;
	//        bm._22 = camDir.z;
	//        bm._32 = 0.0f;
	//        
	//        bm._03 = 0.0f;
	//        bm._13 = 0.0f;
	//        bm._23 = 0.0f;
	//        bm._33 = 1.0f;
	//        
	//        finalMatrix = bm * finalMatrix;
	////        finalMatrix = bm * prevMatrix;
	////        finalMatrix = worldTransform * finalMatrix;
	//    }

	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);

	RenderManager::Instance()->SetRenderData(renderDataObject);
	material->PrepareRenderState();

	RenderManager::Instance()->HWDrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, spriteObject->GetFrame() * 4, 4);
//	material->Draw(dataSource,  materialInstance);


// 	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
// 	RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
// 
// 	eBlendMode sblend = RenderManager::Instance()->GetSrcBlend();
// 	eBlendMode dblend = RenderManager::Instance()->GetDestBlend();
// 
// 	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
// 	RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
// 	RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
// 	RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_WRITE);
// 	//    RenderManager::Instance()->EnableBlending(true);
// 	//    RenderManager::Instance()->EnableTexturing(true);//TODO: Move all this code to the RenderState node
// 	//    RenderManager::Instance()->EnableDepthTest(false);
// 	//    RenderManager::Instance()->EnableDepthWrite(false);
// 
// 
// 	//RenderManager::Instance()->SetState(RenderStateBlock::STATE_BLEND | RenderStateBlock::STATE_TEXTURE0 | RenderStateBlock::STATE_CULL);
// 
// 	int32 frame = spriteObject->GetFrame();
// 	RenderManager::Instance()->SetTexture(spriteObject->GetSprite()->GetTexture(frame));
// 	//	RenderManager::Instance()->FlushState();
// 
// 	RenderManager::Instance()->SetRenderData(renderDataObject);
// 	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLESTRIP, frame * 4, 4);
// 
// 
// 
// 	//    glDisableClientState(GL_COLOR_ARRAY);
// 	RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
// 	//    RenderManager::Instance()->EnableTexturing(true);
// 	//    RenderManager::Instance()->EnableBlending(false);
// 	//    RenderManager::Instance()->EnableDepthTest(true);
// 	//    RenderManager::Instance()->EnableDepthWrite(true);
// 
// 	RenderManager::Instance()->SetBlendMode(sblend, dblend);
// 
// 	//    if (debugFlags & DEBUG_DRAW_ALL)
// 	//    {
// 	//        AABBox3 box(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
// 	//        RenderHelper::Instance()->DrawBox(box);
// 	//    }
// 
// 	RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, modelViewMatrix);
}

const FastName & SpriteRenderBatch::GetOwnerLayerName()
{
	static FastName translucentLayer("TransclucentRenderLayer");
	return translucentLayer;
}



};

