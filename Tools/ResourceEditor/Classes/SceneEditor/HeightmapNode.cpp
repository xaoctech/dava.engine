#include "HeightmapNode.h"
#include "../bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "../EditorScene.h"

#include "Scene3D/Heightmap.h"



HeightmapNode::HeightmapNode(EditorScene * _scene)
    :   SceneNode(_scene)
    ,   position(0,0,0)
    ,   rotation(0)
{
    SetName("HeightmapNode");
    
    editorScene = _scene;
    SetVisible(true);
    
    renderData = NULL;
    
    land = (LandscapeNode*)scene->FindByName("Landscape");
    DVASSERT(land);
    
    Heightmap *heightmap = land->GetHeightmap();

    uint16 *dt = heightmap->Data();
    size.x = heightmap->Size();
    size.y = heightmap->Size();
    for (int32 y = 0; y < heightmap->Size(); y++) 
    {
        for (int32 x = 0; x < heightmap->Size(); x++) 
        {
            hmap.push_back(dt[(x + (y) * heightmap->Size())]);
        }
    }
	
    Vector3 landSize;
    AABBox3 transformedBox;
    land->GetBoundingBox().GetTransformedBox(land->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;
    
    sizeInMeters = landSize.x;
    areaScale = sizeInMeters / heightmap->Size();
    maxHeight = landSize.z;// / 255.f * 65535.f;
    
    
    bool useFloatDatam = false;
	bool flipQuadEdges = true;
    
    
//	colShape = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size()
//                                                                                , &hmap.front(), maxHeight
//                                                                                , 2, useFloatDatam
//                                                                                , flipQuadEdges);

    btScalar heightScale = maxHeight / 65535.f;
    float32 minHeight = 0.0f;
    colShape = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size()
                                             , &hmap.front(), heightScale, minHeight, maxHeight
                                             , 2, PHY_SHORT
                                             , flipQuadEdges);

    
    colShape->setLocalScaling(btVector3(areaScale, areaScale, 1.0f));
    
    
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
                                       btScalar(maxHeight/2.0f + position.z)));
    
    
    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    motionSate = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionSate,colShape,localInertia);
    rbInfo.m_friction = 0.9f;
    body = new btRigidBody(rbInfo);
    
    btVector3 mn;
    btVector3 mx;
    body->getAabb(mn, mx);
    btVector3 sz = mx - mn;
//    Logger::Debug("land size = %f, %f, %f", sz.getX(), sz.getY(), sz.getZ());

    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
    collisionObject->setCollisionShape(colShape);
    editorScene->landCollisionWorld->addCollisionObject(collisionObject);
	editorScene->landCollisionWorld->updateAabbs();
}


HeightmapNode::~HeightmapNode()
{
	if(editorScene->landCollisionWorld)
	{
        editorScene->landCollisionWorld->removeCollisionObject(collisionObject);
	}

    
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

//	Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 
//	Matrix4 meshFinalMatrix = worldTransform * prevMatrix;
//    
//    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
//
//    RenderManager::Instance()->SetRenderEffect(RenderManager::FLAT_COLOR);
//
//    
//    RenderManager::Instance()->SetColor(0.2f, 0.2f, 0.7f, 0.2f);
//    RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
//    RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_DEPTH_TEST|RenderStateBlock::STATE_DEPTH_WRITE);
//    
//	RenderManager::Instance()->FlushState();
//	
//    RenderManager::Instance()->SetRenderData(renderData);
//	RenderManager::Instance()->DrawArrays(PRIMITIVETYPE_TRIANGLELIST, 0, verts.size()/3);
//	
//
//    RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_BLEND);
//    RenderManager::Instance()->AppendState(RenderStateBlock::STATE_DEPTH_TEST|RenderStateBlock::STATE_DEPTH_WRITE);
//    
//    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
}