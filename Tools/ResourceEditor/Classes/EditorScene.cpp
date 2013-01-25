/*
 *  EditorScene.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "EditorScene.h"
#include "SceneNodeUserData.h"
#include "SceneEditor/SceneValidator.h"
#include "Scene3D/LodNode.h"
#include "SceneEditor/EditorSettings.h"
#include "SceneEditor/HeightmapNode.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Render/Highlevel/RenderObject.h"


/*
    This means that if we'll call GameScene->GetClassName() it'll return "Scene"
    This is for correct serialization of framework subclasses.
 */
REGISTER_CLASS_WITH_ALIAS(EditorScene, "Scene");

EditorScene::EditorScene()
:Scene()
{
    selectedEntity = NULL;
    originalHandler = NULL;
	selection = 0;
	lastSelectedPhysics = 0;
	proxy = 0;
    btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
        
    collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	collisionWorld = new btCollisionWorld(dispatcher, broadphase, collisionConfiguration);

	landCollisionConfiguration = new btDefaultCollisionConfiguration();
	landDispatcher = new btCollisionDispatcher(landCollisionConfiguration);
	landBroadphase = new btAxisSweep3(worldMin,worldMax);
	landCollisionWorld = new btCollisionWorld(landDispatcher, landBroadphase, landCollisionConfiguration);

    SetDrawGrid(true);
}

EditorScene::~EditorScene()
{
	ReleaseUserData(this);
	SafeDelete(collisionWorld);
	SafeDelete(broadphase);
	SafeDelete(dispatcher);
	SafeDelete(collisionConfiguration);

	SafeDelete(landCollisionWorld);
	SafeDelete(landBroadphase);
	SafeDelete(landDispatcher);
	SafeDelete(landCollisionConfiguration);
}

void EditorScene::Update(float32 timeElapsed)
{    
    Scene::Update(timeElapsed);
	CheckNodes(this);
	collisionWorld->updateAabbs();
}

void EditorScene::CheckNodes(SceneNode * curr)
{
	MeshInstanceNode * mesh = dynamic_cast<MeshInstanceNode *> (curr);	
	RenderComponent * renderComponent = (RenderComponent*)curr->GetComponent(Component::RENDER_COMPONENT);
	BulletComponent * bulletComponent = (BulletComponent*)curr->GetComponent(Component::BULLET_COMPONENT);
	UserNode * userNode = dynamic_cast<UserNode *> (curr);	
	if(renderComponent && renderComponent->GetRenderObject())
	{
		if (bulletComponent == 0 && curr->IsLodMain(0))
		{
			bulletComponent = (BulletComponent*)curr->GetOrCreateComponent(Component::BULLET_COMPONENT);
			bulletComponent->SetBulletObject(ScopedPtr<BulletObject>(new BulletObject(this, collisionWorld, curr, curr->GetWorldTransform())));
		}
		else if(bulletComponent && bulletComponent->GetBulletObject())
		{
			((BulletObject*)bulletComponent->GetBulletObject())->UpdateCollisionObject();
		}
	}
	//else if (userNode)
	//{
	//	if (userNode->GetUserData() == 0)
	//	{
	//		SceneNodeUserData * data = new SceneNodeUserData();
	//		curr->SetUserData(data);
	//		data->bulletObject = new BulletObject(this, collisionWorld, userNode, userNode->GetWorldTransform());
	//		SafeRelease(data);
	//	}
	//	else if (userNode->GetUserData())
	//	{
	//		SceneNodeUserData * data = (SceneNodeUserData*)userNode->GetUserData();
	//		data->bulletObject->UpdateCollisionObject();
	//	}
	//}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		CheckNodes(curr->GetChild(i));
	}
}

void EditorScene::ReleaseUserData(SceneNode * curr)
{
	if(curr)
	{
		curr->RemoveComponent(Component::BULLET_COMPONENT);
	}

	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		ReleaseUserData(curr->GetChild(i));
	}
}

SceneNode * GetSolidParent(SceneNode* curr)
{
	if (curr == 0)
		return 0;
	
	SceneNode * parentResult = GetSolidParent(curr->GetParent());
	
	if (curr->GetSolid() && parentResult == 0)
	{
		return curr;
	}
	else
		return parentResult;
}

SceneNode * GetLodParent(SceneNode * curr)
{
	bool hasLod = (curr->GetComponent(Component::LOD_COMPONENT) != 0);
	if(hasLod)
	{
		return curr;
	}
	else 
	{
		SceneNode * parent = curr->GetParent();
		if (parent == 0)
			return 0;
		return GetLodParent(parent);
	}
}



void EditorScene::TrySelection(Vector3 from, Vector3 direction)
{
	if (selection)
		selection->SetDebugFlags(selection->GetDebugFlags() & (~DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS));

	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(direction.x, direction.y, direction.z);
		
    btCollisionWorld::AllHitsRayResultCallback cb(pos, to);
    collisionWorld->rayTest(pos, to, cb);
	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		//Logger::Debug("Has Hit");
		int findedIndex = cb.m_collisionObjects.size() - 1;
		if(lastSelectedPhysics)
		{
			BulletComponent * bulletComponent = (BulletComponent*)lastSelectedPhysics->GetComponent(Component::BULLET_COMPONENT);
			BulletObject * bulletObject = (BulletObject*)bulletComponent->GetBulletObject();
			if (bulletObject)
			{
				for (int i = cb.m_collisionObjects.size() - 1; i >= 0 ; i--)
				{					
					if (bulletObject->GetCollisionObject() == cb.m_collisionObjects[i])
					{
						findedIndex = i;
						break;
					}
				}
				while (findedIndex >= 0 && bulletObject->GetCollisionObject() == cb.m_collisionObjects[findedIndex])
					findedIndex--;
				findedIndex = findedIndex % cb.m_collisionObjects.size();
			}
		}
			//		Logger::Debug("size:%d selIndex:%d", cb.m_collisionObjects.size(), findedIndex);
		
		if (findedIndex == -1)
			findedIndex = cb.m_collisionObjects.size() - 1;
		coll = cb.m_collisionObjects[findedIndex];
		selection = FindSelected(this, coll);
		lastSelectedPhysics = selection;
		SetSelection(selection);
	}
	else 
	{
		SetSelection(0);
	}
}

void EditorScene::JuncCollWorldToLandscapeCollWorld()
{
	
	btCollisionObjectArray& landscapeObjects = landCollisionWorld->getCollisionObjectArray();
	if(landscapeObjects.size() > 1)
	{
		return;
	}

	for(int32 i = 0; i < landscapeObjects.size(); ++i)
	{
		originalHandler = landscapeObjects[i]->getBroadphaseHandle();
		collisionWorld->addCollisionObject(landscapeObjects[i]);
	}
}

void EditorScene::SeparateCollWorldFromLandscapeCollWorld()
{
	btCollisionObjectArray& landscapeObjects = landCollisionWorld->getCollisionObjectArray();
	if(landscapeObjects.size() > 1)
	{
		return;
	}
	for(int32 i = 0; i < landscapeObjects.size(); ++i)
	{
		collisionWorld->removeCollisionObject(landscapeObjects[i]);
		landscapeObjects[i]->setBroadphaseHandle(originalHandler);
	}
}

bool EditorScene::TryIsTargetAccesible(Vector3 from, Vector3 target)
{
	if (selection)
		selection->SetDebugFlags(selection->GetDebugFlags() & (~DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS));

	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(target.x, target.y, target.z);

    btCollisionWorld::AllHitsRayResultCallback cb(pos, to);
    
	collisionWorld->rayTest(pos, to, cb);

	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		//there is some obj before target
		//cb.m_hitPointWorld - contain coord of breaker
		return false;
	}
	else
	{
		return true;
	}
	
}

bool EditorScene::LandscapeIntersection(const DAVA::Vector3 &from, const DAVA::Vector3 &direction, DAVA::Vector3 &point)
{
	btVector3 pos(from.x, from.y, from.z);
    btVector3 to(direction.x, direction.y, direction.z);
    
    btCollisionWorld::ClosestRayResultCallback cb(pos, to);
	uint64 time1 = SystemTimer::Instance()->AbsoluteMS();
	uint64 time2;
    landCollisionWorld->rayTest(pos, to, cb);
	time2 = SystemTimer::Instance()->AbsoluteMS();
	//Logger::Debug("raytest %lld", time2-time1);
	btCollisionObject * coll = 0;
	if (cb.hasHit()) 
    {
		//int findedIndex = cb.m_collisionObjects.size() - 1;
		//
		//if (findedIndex == -1)
		//	findedIndex = cb.m_collisionObjects.size() - 1;
		//coll = cb.m_collisionObjects[findedIndex];
        
		//HeightmapNode *hm = FindHeightmap(this, coll);
        //if(hm)
        {
            point.x = cb.m_hitPointWorld.x();
            point.y = cb.m_hitPointWorld.y();
            point.z = cb.m_hitPointWorld.z();
            
            return true;
        }
    }
    
    return false;
}



LandscapeNode * EditorScene::GetLandScape(SceneNode *node)
{
	RenderComponent* renderComponent = cast_if_equal<RenderComponent*>(node->GetComponent(Component::RENDER_COMPONENT));
	if (renderComponent)
	{
		LandscapeNode* land = dynamic_cast<LandscapeNode*>(renderComponent->GetRenderObject());
		if (land)
			return land;
	}
	
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        SceneNode * child = node->GetChild(ci);
		LandscapeNode * result = GetLandScape(child);
		if (result)
			return result;
    }
	return 0;
}

SceneNode* EditorScene::GetLandScapeNode(SceneNode *node)
{
	RenderComponent* renderComponent = cast_if_equal<RenderComponent*>(node->GetComponent(Component::RENDER_COMPONENT));
	if (renderComponent)
	{
		LandscapeNode* land = dynamic_cast<LandscapeNode*>(renderComponent->GetRenderObject());
		if (land)
			return node;
	}
	
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        SceneNode * child = node->GetChild(ci);
		SceneNode * result = GetLandScapeNode(child);
		if (result)
			return result;
    }
	return NULL;
}

HeightmapNode * EditorScene::FindHeightmap(SceneNode * curr, btCollisionObject * coll)
{
	HeightmapNode * node = dynamic_cast<HeightmapNode *> (curr);
	if (node && node->collisionObject == coll)
	{
        return node;
	}
    
	int size = curr->GetChildrenCount();
//	for (int i = 0; i < size; i++)
	for (int i = size-1; i >= 0; --i)
	{
		HeightmapNode * result = FindHeightmap(curr->GetChild(i), coll);
		if (result)
			return result;
	}
	return 0;
}


SceneNode * EditorScene::FindSelected(SceneNode * curr, btCollisionObject * coll)
{
	SceneNode * node = curr;
// LIGHT
//	if (node == 0)
//		node = dynamic_cast<LightNode *> (curr);
    
	if (node == 0)
		node = dynamic_cast<UserNode *> (curr);
	
	BulletComponent * bulletComponent = (BulletComponent*)node->GetComponent(Component::BULLET_COMPONENT);
	if (bulletComponent && bulletComponent->GetBulletObject())
	{
		BulletObject * bulletObject = (BulletObject*)bulletComponent->GetBulletObject();
		if (bulletObject->GetCollisionObject() == coll)
			return curr;
	}
    
	int size = curr->GetChildrenCount();
	for (int i = 0; i < size; i++)
	{
		SceneNode * result = FindSelected(curr->GetChild(i), coll);
		if (result)
			return result;
	}
	return 0;
}

void EditorScene::SetSelection(SceneNode *newSelection)
{
    if (selection)
    {
        uint32 flags = selection->GetDebugFlags();
        uint32 newFlags = flags & ~DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS;
        
        SetNodeDebugFlags(selection, newFlags);
    }
    
	selection = newSelection;
    selectedEntity = newSelection;
    
	if (selection)
	{
		SceneNode * solid = GetSolidParent(selection);
		if(solid == 0)
		{
			solid = GetLodParent(selection);
		}
		if(solid)
		{
			selection = solid;
		}
			
		
		proxy = GetHighestProxy(selection);
		if (proxy == 0)
			proxy = selection;
	}
	else
	{
		proxy = 0;
		lastSelectedPhysics = 0;
	}

	if(selection)
    {
        uint32 flags = selection->GetDebugFlags();
        uint32 newFlags = flags | DebugRenderComponent::DEBUG_DRAW_AABOX_CORNERS;
        
        SetNodeDebugFlags(selection, newFlags);
    }
}

void EditorScene::SetNodeDebugFlags(SceneNode *selectedNode, uint32 flags)
{
    selectedNode->SetDebugFlags(flags);
    if(selectedEntity && selectedEntity != selectedNode)
    {
        selectedEntity->SetDebugFlags(flags, false);
    }
}


SceneNode * EditorScene::GetHighestProxy(SceneNode* curr)
{
    if(!curr) return NULL;
    
	int32 cc = curr->GetChildrenCount();
	if (cc == 0)
		return GetHighestProxy(curr->GetParent());
	if (cc > 1)
		return 0;
	if (cc == 1)
    {
        SceneNode * result = GetHighestProxy(curr->GetParent());
	    if (result == 0)
            return curr;
        else return result;
        
    }
    return NULL;
}

void EditorScene::SetBulletUpdate(SceneNode* curr, bool value)
{
	if(lastSelectedPhysics)
	{
		BulletComponent * bulletComponent = (BulletComponent*)lastSelectedPhysics->GetComponent(Component::BULLET_COMPONENT);
		if (bulletComponent)
		{
			BulletObject * bulletObject = (BulletObject*)bulletComponent->GetBulletObject();
			bulletObject->SetUpdateFlag(value);
		}
	}
	
	for (int32 i = 0; i < curr->GetChildrenCount(); i++)
	{
		SetBulletUpdate(curr->GetChild(i), value);
	}
}


void EditorScene::Draw()
{
//	DrawDebugNodes(this);
    
    RenderManager::Instance()->ClearStats();
	Scene::Draw();
    SceneValidator::Instance()->CollectSceneStats(RenderManager::Instance()->GetStats());
    
    if(drawGrid)
    {
        DrawGrid();
    }
}

#define GRIDMAX 500.0f
#define GRIDSTEP 10.0f
void EditorScene::DrawGrid()
{
    uint32 oldState = RenderManager::Instance()->GetState();	
	RenderManager::Instance()->SetState(RenderStateBlock::STATE_COLORMASK_ALL | RenderStateBlock::STATE_DEPTH_WRITE | RenderStateBlock::STATE_DEPTH_TEST); 
	RenderManager::Instance()->SetColor(0.7f, 0.7f, 0.7f, 1.0f);
	for (float32 x = -GRIDMAX; x <= GRIDMAX; x+=GRIDSTEP)
	{
		Vector3 v1(x, -GRIDMAX, 0);
		Vector3 v2(x, GRIDMAX, 0);
		
		Vector3 v3(-GRIDMAX, x, 0);
		Vector3 v4(GRIDMAX, x, 0);
		if (x!= 0.0f)
		{
			RenderHelper::Instance()->DrawLine(v1, v2);
			RenderHelper::Instance()->DrawLine(v3, v4);		
		}
	}
	RenderManager::Instance()->SetColor(0.0f, 0.0f, 0.0f, 1.0f);
	RenderHelper::Instance()->DrawLine(Vector3(-GRIDMAX, 0, 0), Vector3(GRIDMAX, 0, 0));
	RenderHelper::Instance()->DrawLine(Vector3(0, -GRIDMAX, 0), Vector3(0, GRIDMAX, 0));
	RenderManager::Instance()->SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	
	RenderManager::Instance()->SetState(oldState);
}

void EditorScene::SetDrawGrid(bool newDrawGrid)
{
    drawGrid = newDrawGrid;
}

void EditorScene::SetForceLodLayer(SceneNode *node, int32 layer)
{
    if(!node)   return;
    
    SceneNode *n = node;
    
    do {
        LodNode *lodNode = dynamic_cast<LodNode *>(n);
        if(lodNode)
        {
            lodNode->SetForceLodLayer(layer);
        }
        
        n = n->GetParent();
    } while (n);
    
    SetForceLodLayerRecursive(node, layer);
}

void EditorScene::SetForceLodLayerRecursive(SceneNode *node, int32 layer)
{
    LodNode *lodNode = dynamic_cast<LodNode *>(node);
    if(lodNode)
    {
        lodNode->SetForceLodLayer(layer);
    }
    
    int32 count = node->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        SetForceLodLayerRecursive(node->GetChild(i), layer);
    }
}


int32 EditorScene::GetForceLodLayer(SceneNode *node)
{
    if(!node)   return -1;

    LodNode *lodNode = dynamic_cast<LodNode *>(node);
    if(lodNode)
        return lodNode->GetForceLodLayer();
    
    int32 count = node->GetChildrenCount();
    for(int32 i = 0; i < count; ++i)
    {
        int32 layer = GetForceLodLayer(node->GetChild(i));
        if(-1 != layer)
            return layer;
    }
    
    return -1;
}
