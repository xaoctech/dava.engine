#include "HeightmapNode.h"
#include "bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "../EditorScene.h"

HeightmapNode::HeightmapNode(EditorScene * _scene)
    :   SceneNode(_scene)
    ,   position(0,0,0)
    ,   rotation(0)
{
    editorScene = _scene;
    SetVisible(true);
    
    renderData = NULL;
    
    land = (LandscapeNode*)scene->FindByName("Landscape");
    
    heightMap = Image::CreateFromFile(land->GetHeightMapPathname());

    uint8 *dt = (uint8 *)heightMap->GetData();
    size.x = heightMap->GetWidth();
    size.y = heightMap->GetHeight();
    for (int32 y = 0; y < heightMap->GetHeight(); y++) 
    {
        for (int32 x = 0; x < heightMap->GetWidth(); x++) 
        {
            hmap.push_back(dt[(x + (y) * heightMap->GetWidth())]);
        }
    }
	
    Vector3 landSize;
    AABBox3 transformedBox;
    land->GetBoundingBox().GetTransformedBox(land->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;
    
    sizeInMeters = landSize.x;
    areaScale = sizeInMeters / heightMap->GetWidth();
    maxHeight = landSize.z / 255.f * 65535.f;

    
    bool useFloatDatam = false;
	bool flipQuadEdges = true;
    
    
	colShape = new btHeightfieldTerrainShape(heightMap->GetWidth(), heightMap->GetHeight()
                                                                                , &hmap.front(), maxHeight
                                                                                , 2, useFloatDatam
                                                                                , flipQuadEdges);
    
    colShape->setLocalScaling(btVector3(areaScale, areaScale, 1.0));
    
    
    /// Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();
    
    btScalar	mass(0.0f);
    
		//rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);
    
    btVector3 localInertia(0,0,0);
    if (isDynamic)
        colShape->calculateLocalInertia(mass,localInertia);
    
    startTransform.setOrigin(btVector3( btScalar(position.x),
                                       btScalar(position.y),
                                       btScalar(maxHeight/2 + position.z)));
    
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    motionSate = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionSate,colShape,localInertia);
    rbInfo.m_friction = 0.9;
    body = new btRigidBody(rbInfo);
    
//    editorScene->dynamicsWorld->addRigidBody(body);

    btVector3 mn;
    btVector3 mx;
    body->getAabb(mn, mx);
    btVector3 sz = mx - mn;
    Logger::Debug("land size = %f, %f, %f", sz.getX(), sz.getY(), sz.getZ());

    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
    collisionObject->setCollisionShape(colShape);
    editorScene->collisionWorld->addCollisionObject(collisionObject);
    

    SafeRelease(heightMap);
}


HeightmapNode::~HeightmapNode()
{
    editorScene->collisionWorld->removeCollisionObject(collisionObject);
    
    SafeDelete(body);
    
    SafeDelete(collisionObject);
    
    SafeDelete(motionSate);
    
    SafeDelete(colShape);
    
    SafeRelease(renderData);
}



float32 HeightmapNode::GetAreaScale()
{
    return areaScale;
}

const Vector3 &HeightmapNode::GetSize()
{
    return size;
}

float32 HeightmapNode::GetSizeInMeters()
{
    return sizeInMeters;
}



void HeightmapNode::Draw()
{
	if (!GetVisible())return;
    SceneNode::Draw();

    return;

	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);

    RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);

    
    RenderManager::Instance()->SetColor(0.2, 0.2, 0.7, 0.2);
    RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
    RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_TEST|RenderStateBlock::STATE_DEPTH_WRITE);
    
	RenderManager::Instance()->FlushState();
	
    RenderManager::Instance()->SetRenderData(renderData);
	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, verts.size()/3);
	

    RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_BLEND);
    RenderManager::Instance()->AppendState(RenderStateBlock::STATE_DEPTH_TEST|RenderStateBlock::STATE_DEPTH_WRITE);
    
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
}