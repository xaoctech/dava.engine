#include "Entity/Component.h"
#include "Scene3D/Systems/DebugRenderSystem.h"
#include "Debug/DVAssert.h"
#include "Scene3D/SceneNode.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"

#include "Render/Highlevel/Camera.h"
#include "Render/RenderHelper.h"

namespace DAVA
{

DebugRenderSystem::DebugRenderSystem()
    : camera(0)
{
}
    
DebugRenderSystem::~DebugRenderSystem()
{
    
}

void DebugRenderSystem::Process()
{
    uint32 size = entities.size();
	for(uint32 i = 0; i < size; ++i)
	{
        SceneNode * entity = entities[i];
        
        DebugRenderComponent * debugRenderComponent = cast_if_equal<DebugRenderComponent*>(entity->GetComponent(Component::DEBUG_RENDER_COMPONENT));
        TransformComponent * transformComponent = cast_if_equal<TransformComponent*>(entity->GetComponent(Component::TRANSFORM_COMPONENT));
        RenderComponent * renderComponent = cast_if_equal<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
        
        Matrix4 worldTransform = /*(*transformComponent->GetWorldTransform()) * */camera->GetMatrix();
        RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, camera->GetMatrix());

        
        AABBox3 box = entity->GetWTMaximumBoundingBoxSlow();
        
        uint32 debugFlags = debugRenderComponent->GetDebugFlags();
        
        if ((debugFlags & DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS))
        {            
            RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
            RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST);
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            RenderHelper::Instance()->DrawCornerBox(box);
            RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            //		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
        }
        
        if (debugFlags & DebugRenderComponent::DEBUG_DRAW_RED_AABBOX)
        {
            RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
            RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST);
            RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            RenderHelper::Instance()->DrawBox(box);
            RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
        
        // Camera debug draw
#if 0
        if (debugFlags & DEBUG_DRAW_ALL)
        {
            Camera * prevCamera = scene->GetCurrentCamera();
            
            // Build this camera matrixes & it's frustum
            this->Set();
            
            // Restore original camera
            // Do that only if exists because potentially it can be scene without camera set
            if (prevCamera)
                prevCamera->Set();
            
            RenderManager::Instance()->SetColor(0.0f, 1.0f, 0.0f, 1.0f);
            
            // If this is clip camera - show it as red camera
            if (this == scene->GetClipCamera())
                RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            
            // Draw frustum of this camera
            if (currentFrustum)
            {
                currentFrustum->DebugDraw();
            }
            // reset color
            RenderManager::Instance()->ResetColor();
        }
#endif
        
        // LightNode debug draw
#if 0
        if (debugFlags != DEBUG_DRAW_NONE)
        {
            if (!(flags & SceneNode::NODE_VISIBLE))return;
            
            RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
            RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE);
            
            RenderManager::Instance()->SetColor(1.0f, 0.0f, 0.0f, 1.0f);
            RenderHelper::Instance()->DrawLine(position, position + direction * 20);
            
            RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
#endif
        // UserNode Draw
#if 0
       	
        if (debugFlags & DEBUG_DRAW_USERNODE)
        {
            Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW);
            Matrix4 finalMatrix = worldTransform * prevMatrix;
            RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
            
            RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
            RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST);
            RenderManager::Instance()->SetColor(0, 0, 1.0f, 1.0f);
            RenderHelper::Instance()->DrawBox(drawBox);
            RenderManager::Instance()->SetColor(1.f, 1.f, 0, 1.0f);
            RenderHelper::Instance()->DrawLine(Vector3(0, 0, 0), Vector3(1.f, 0, 0));
            RenderManager::Instance()->SetColor(1.f, 0, 1.f, 1.0f);
            RenderHelper::Instance()->DrawLine(Vector3(0, 0, 0), Vector3(0, 1.f, 0));
            RenderManager::Instance()->SetColor(0, 1.f, 1.f, 1.0f);
            RenderHelper::Instance()->DrawLine(Vector3(0, 0, 0), Vector3(0, 0, 1.f));
            RenderManager::Instance()->SetState(RenderStateBlock::DEFAULT_3D_STATE);
            RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
            RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
        }
#endif
        
#if 0
        // ParticleEffectNode
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
#endif
        
    }
}

void DebugRenderSystem::AddEntity(SceneNode * entity)
{
	entities.push_back(entity);

    DebugRenderComponent * debugRenderComponent = static_cast<DebugRenderComponent*>(entity->GetComponent(Component::DEBUG_RENDER_COMPONENT));
    RenderComponent * renderComponent = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
    if (renderComponent)
    {
        //renderComponent->renderObject->SetDebugFlags(debugRenderComponent->GetFlags());
    }
}

void DebugRenderSystem::RemoveEntity(SceneNode * entity)
{
    DebugRenderComponent * debugRenderComponent = static_cast<DebugRenderComponent*>(entity->GetComponent(Component::DEBUG_RENDER_COMPONENT));
    RenderComponent * renderComponent = static_cast<RenderComponent*>(entity->GetComponent(Component::RENDER_COMPONENT));
    if (renderComponent)
    {
        //renderComponent->renderObject->SetDebugFlags(0);
    }

	
    uint32 size = entities.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(entities[i] == entity)
		{
			entities[i] = entities[size-1];
			entities.pop_back();
			return;
		}
	}
	
	DVASSERT(0);
}
    
void DebugRenderSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

}
