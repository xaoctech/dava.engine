#include "HeightmapNode.h"
#include "../bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include "../EditorScene.h"
#include "Scene3D/Heightmap.h"

#include "Scene3D/LandscapeCursor.h"

HeightmapNode::HeightmapNode(EditorScene * _scene, LandscapeNode *_land)
    :   SceneNode()
    ,   position(0,0,0)
    ,   rotation(0)
{
    SetName("HeightmapNode");
    
    editorScene = _scene;
    SetVisible(false);

    land = _land;
    DVASSERT(land);
    
    //Get LandSize;
    Vector3 landSize;
    AABBox3 transformedBox;
    land->GetBoundingBox().GetTransformedBox(land->GetWorldTransform(), transformedBox);
    landSize = transformedBox.max - transformedBox.min;
    
    heightmap = land->GetHeightmap();
    sizeInMeters = landSize.x;
    areaScale = sizeInMeters / heightmap->Size();
    maxHeight = landSize.z;
    heightScale = maxHeight / 65535.f;
    float32 minHeight = 0.0f;

    heightmapTexture = Texture::CreateFromFile("/Users/klesch/Work/WoT/Framework/wot.sniper/DataSource/lm_l2.png");//heit_l2.png");
    debugVertices.resize(heightmap->Size() * heightmap->Size());
	debugIndices.resize(heightmap->Size() * heightmap->Size() * 6);

    renderDataObject = new RenderDataObject();
	renderDataObject->SetStream(EVF_VERTEX, TYPE_FLOAT, 3, sizeof(LandscapeNode::LandscapeVertex), &debugVertices[0].position); 
	renderDataObject->SetStream(EVF_TEXCOORD0, TYPE_FLOAT, 2, sizeof(LandscapeNode::LandscapeVertex), &debugVertices[0].texCoord); 

	int32 step = 1;
	int32 indexIndex = 0;
	int32 quadWidth = heightmap->Size();
	for(int32 y = 0; y < heightmap->Size() - 1; y += step)
	{
		for(int32 x = 0; x < heightmap->Size() - 1; x += step)
		{
			debugIndices[indexIndex++] = x + y * quadWidth;
			debugIndices[indexIndex++] = (x + step) + y * quadWidth;
			debugIndices[indexIndex++] = x + (y + step) * quadWidth;
            
			debugIndices[indexIndex++] = (x + step) + y * quadWidth;
			debugIndices[indexIndex++] = (x + step) + (y + step) * quadWidth;
			debugIndices[indexIndex++] = x + (y + step) * quadWidth;     
		}
	}
    
    
    uint16 *dt = heightmap->Data();
    size.x = heightmap->Size();
    size.y = heightmap->Size();
    hmap.resize(heightmap->Size() * heightmap->Size());

	for (int32 y = 0; y < heightmap->Size(); ++y)
	{
		for (int32 x = 0; x < heightmap->Size(); ++x)
		{
            int32 index = x + (y) * heightmap->Size();
            float32 mapValue = dt[index] * heightScale;
            SetValueToMap(x, y, mapValue, transformedBox);
            
			debugVertices[index].texCoord = Vector2((float32)x / (float32)(heightmap->Size() - 1), 
                                                    (float32)y / (float32)(heightmap->Size() - 1));           
		}
	}

	bool flipQuadEdges = true;

    colShape = new btHeightfieldTerrainShape(heightmap->Size(), heightmap->Size()
                                             , &hmap.front(), heightScale, minHeight, maxHeight
                                             , 2, PHY_FLOAT
                                             , flipQuadEdges);

    colShape->setLocalScaling(btVector3(areaScale, areaScale, 1.0f));
    
    // Create Dynamic Objects
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
    
//    btVector3 mn;
//    btVector3 mx;
//    body->getAabb(mn, mx);
//    btVector3 sz = mx - mn;
//    Logger::Debug("land size = %f, %f, %f", sz.getX(), sz.getY(), sz.getZ());

    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
    collisionObject->setCollisionShape(colShape);
    editorScene->landCollisionWorld->addCollisionObject(collisionObject);
	editorScene->landCollisionWorld->updateAabbs();
    
    cursor = new LandscapeCursor();
}

HeightmapNode::~HeightmapNode()
{
    SafeDelete(cursor);
    
	if(editorScene->landCollisionWorld)
	{
        editorScene->landCollisionWorld->removeCollisionObject(collisionObject);
	}
    
    
    SafeDelete(body);
    
    SafeDelete(collisionObject);
    
    SafeDelete(motionSate);
    
    SafeDelete(colShape);
    
    SafeRelease(heightmapTexture);
    
    SafeRelease(renderDataObject);
}


void HeightmapNode::SetValueToMap(int16 x, int16 y, float32 height, const AABBox3 &box)
{
    int32 index = x + y * heightmap->Size();

    hmap[index] = height;
    debugVertices[index].position = GetPoint(x, y, height, box);
}

void HeightmapNode::UpdateHeightmapRect(const Rect &rect)
{
    AABBox3 transformedBox;
    land->GetBoundingBox().GetTransformedBox(land->GetWorldTransform(), transformedBox);
    
    int32 x = Max(0.f, rect.x);
    int32 y = Max(0.f, rect.y);
    int32 endX = Min(rect.x + rect.dx, heightmap->Size() - 1.0f);
    int32 endY = Min(rect.y + rect.dy, heightmap->Size() - 1.0f);

    uint16 *dt = heightmap->Data();
    for (int32 yy = y; yy < endY; ++yy)
    {
		for (int32 xx = x; xx < endX; ++xx)
		{
            float32 mapValue = dt[(xx + (yy) * heightmap->Size())] * heightScale;
            SetValueToMap(xx, yy, mapValue, transformedBox);
		}
	}
}


Vector3 HeightmapNode::GetPoint(int16 x, int16 y, float32 height, const AABBox3 &box)
{
    Vector3 res;
    res.x = (box.min.x + (float32)x / (float32)(heightmap->Size() - 1) * (box.max.x - box.min.x));
    res.y = (box.min.y + (float32)y / (float32)(heightmap->Size() - 1) * (box.max.y - box.min.y));
    res.z = (box.min.z + height);
    return res;
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

    RenderManager::Instance()->SetRenderEffect(RenderManager::TEXTURE_MUL_FLAT_COLOR);
    RenderManager::Instance()->SetTexture(heightmapTexture, 0);

    RenderManager::Instance()->SetRenderData(renderDataObject);
	RenderManager::Instance()->FlushState();
	RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, &debugIndices.front()); 
    
    if(cursor)
	{
		RenderManager::Instance()->AppendState(RenderStateBlock::STATE_BLEND);
		eBlendMode src = RenderManager::Instance()->GetSrcBlend();
		eBlendMode dst = RenderManager::Instance()->GetDestBlend();
		RenderManager::Instance()->SetBlendMode(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		RenderManager::Instance()->SetDepthFunc(CMP_LEQUAL);
		cursor->Prepare();
        
		RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, (heightmap->Size() - 1) * (heightmap->Size() - 1) * 6, EIF_32, &debugIndices.front()); 
        
		RenderManager::Instance()->SetDepthFunc(CMP_LESS);
		RenderManager::Instance()->RemoveState(RenderStateBlock::STATE_BLEND);
		RenderManager::Instance()->SetBlendMode(src, dst);
	}
}