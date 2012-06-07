#include "Base/BaseMath.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Entity/ComponentDataMapper.h"
#include "Scene3D/Camera.h"

using namespace DAVA;


//class ComponentVar;

DECLARE_COMPONENT(VisibilityAABBoxComponent);  
//DECLARE_COMPONENT_VAR(VisibilityAABBoxComponent, AABBox3, MeshAABBoxData);

void VisibilityAABBoxComponent::Register()
{   
    RegisterData<AABBox3>("meshAABox");
	RegisterData<uint32>("meshVisibilityFlag");
}

DECLARE_COMPONENT(VisibilityBSphereComponent);  
void VisibilityBSphereComponent::Register()
{   
	RegisterData<Sphere>("meshBSphere");
	RegisterData<uint32>("meshVisibilityFlag");
}

DECLARE_COMPONENT(DrawMeshComponent);
void DrawMeshComponent::Register()
{
	//RegisterData<void*>("mesh");
	RegisterData<uint32>("meshVisibilityFlag");
	RegisterData<Matrix4>("worldTransform");
}


int32 testResults[16];

class VisibilityAABBoxSystem
{
public:
	EntityManager * manager;
	//DataIndex meshVisibilityFlag;

	//const char *, Pool *
	//const char *, int
	//int, Pool*

	//VisibilityAABBoxSystem()
	//	meshVisibilityFlag("meshVisibilityFlag")
	//{

	//}


	void Run()
	{
		TemplatePool<AABBox3> * boxes = manager->GetLinkedTemplatePools<AABBox3>("meshAABox");
		while(boxes)
		{
			EntityFamily * family = boxes->GetEntityFamily();
			TemplatePool<uint32> * visibilityFlags = (TemplatePool<uint32>*)family->GetPoolByDataName("meshVisibilityFlag");

			int32 count = boxes->GetCount();
			AABBox3 * box = boxes->GetPtr(0);
			uint32 * flag = visibilityFlags->GetPtr(0);

			for(int32 i = 0; i < count; ++i)
			{
				*flag = box->max.x > 0;

				box++;
				flag++;
			}

			boxes = boxes->next;
		}
	}
};

class VisibilityBSphereSystem
{
public:
	EntityManager * manager;

	void Run()
	{
		TemplatePool<Sphere> * spheres = manager->GetLinkedTemplatePools<Sphere>("meshBSphere");
		while(spheres)
		{
			EntityFamily * family = spheres->GetEntityFamily();
			TemplatePool<uint32> * visibilityFlags = (TemplatePool<uint32>*)family->GetPoolByDataName("meshVisibilityFlag");

			int32 count = spheres->GetCount();
			Sphere * sphere = spheres->GetPtr(0);
			uint32 * flag = visibilityFlags->GetPtr(0);

			for(int32 i = 0; i < count; ++i)
			{
				*flag = sphere->center.x > 0;

				sphere++;
				flag++;
			}

			spheres = spheres->next;
		}
	}
};

class DrawMeshSystem
{
public:
	EntityManager * manager;
    
    void Run()
    {
		TemplatePool<uint32> * visibilityFlags = manager->GetLinkedTemplatePools<uint32>("meshVisibilityFlag");

		int32 * result = testResults;
		while(visibilityFlags)
		{
			int32 count = visibilityFlags->GetCount();
			uint32 * flag = visibilityFlags->GetPtr(0);
			for(int32 i = 0; i < count; ++i)
			{
				*result = *flag;

				result++;
				flag++;
			}

			visibilityFlags = visibilityFlags->next;
		}
    }
    
};

void EntityTest()
{
    VisibilityAABBoxComponent::Create();
    VisibilityBSphereComponent::Create();
    DrawMeshComponent::Create();

	EntityManager * manager = new EntityManager();

	VisibilityAABBoxSystem visibilityAABBoxSystem;
	visibilityAABBoxSystem.manager = manager;

	DrawMeshSystem drawSystem;
	drawSystem.manager = manager;

	VisibilityBSphereSystem visibilityBSphereSystem;
	visibilityBSphereSystem.manager = manager;

	Entity * entity0 = EntityManager::Instance()->CreateEntity();
	entity0->AddComponent(VisibilityAABBoxComponent::Get());
	entity0->AddComponent(DrawMeshComponent::Get());

	Entity * entity1 = EntityManager::Instance()->CreateEntity();
	entity1->AddComponent(VisibilityAABBoxComponent::Get());
	entity1->AddComponent(DrawMeshComponent::Get());

	Entity * entity2 = EntityManager::Instance()->CreateEntity();
	entity2->AddComponent(VisibilityAABBoxComponent::Get());
	entity2->AddComponent(DrawMeshComponent::Get());

	Entity * entity3 = EntityManager::Instance()->CreateEntity();
	entity3->AddComponent(VisibilityAABBoxComponent::Get());
	entity3->AddComponent(DrawMeshComponent::Get());

	Entity * entity4 = EntityManager::Instance()->CreateEntity();
	entity4->AddComponent(VisibilityBSphereComponent::Get());
	entity4->AddComponent(DrawMeshComponent::Get());
	Sphere sphere4;
	sphere4.center = Vector3(1.f, 1.f, 1.f);

	Entity * entity5 = EntityManager::Instance()->CreateEntity();
	entity5->AddComponent(VisibilityBSphereComponent::Get());
	entity5->AddComponent(DrawMeshComponent::Get());
	Sphere sphere5;
	sphere5.center = Vector3(-1.f, -1.f, -1.f);

	manager->Flush();

	entity0->SetData("meshAABox", AABBox3(Vector3(-5.f, -5.f, -5.f), Vector3(-4.f, -4.f, -4.f)));
	entity0->SetData("meshVisibilityFlag", (uint32)0);
	entity1->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity1->SetData("meshVisibilityFlag", (uint32)0);
	entity2->SetData("meshAABox", AABBox3(Vector3(-2.f, -2.f, -2.f), Vector3(-1.f, -1.f, -1.f)));
	entity2->SetData("meshVisibilityFlag", (uint32)0);
	entity3->SetData("meshAABox", AABBox3(Vector3(3.f, 3.f, 3.f), Vector3(4.f, 4.f, 4.f)));
	entity3->SetData("meshVisibilityFlag", (uint32)0);
	entity4->SetData("meshBSphere", sphere4);
	entity4->SetData("meshVisibilityFlag", (uint32)0);
	entity5->SetData("meshBSphere", sphere5);
	entity5->SetData("meshVisibilityFlag", (uint32)0);

	visibilityAABBoxSystem.Run();
	visibilityBSphereSystem.Run();
	drawSystem.Run();

	int32 expectedResultsAdd[6] = {1,0,0,1,0,1};
	for(int32 i = 0; i < 6; ++i)
	{
		DVASSERT(testResults[i] == expectedResultsAdd[i]);
	}

	entity2->RemoveComponent(VisibilityAABBoxComponent::Get());

	manager->Flush();

	entity0->SetData("meshVisibilityFlag", (uint32)0);
	entity1->SetData("meshVisibilityFlag", (uint32)0);
	entity2->SetData("meshVisibilityFlag", (uint32)0);
	entity3->SetData("meshVisibilityFlag", (uint32)0);
	entity4->SetData("meshVisibilityFlag", (uint32)0);
	entity5->SetData("meshVisibilityFlag", (uint32)0);

	visibilityAABBoxSystem.Run();
	visibilityBSphereSystem.Run();
	drawSystem.Run();

	manager->Dump();

	int32 expectedResultsRemove[6] = {0,1,0,0,1,1};
	for(int32 i = 0; i < 6; ++i)
	{
		DVASSERT(testResults[i] == expectedResultsRemove[i]);
	}

	Entity * entity6 = EntityManager::Instance()->CreateEntity();
	entity6->AddComponent(VisibilityAABBoxComponent::Get());
	entity6->AddComponent(DrawMeshComponent::Get());

	Entity * entity7 = EntityManager::Instance()->CreateEntity();
	entity7->AddComponent(VisibilityAABBoxComponent::Get());
	entity7->AddComponent(DrawMeshComponent::Get());

	Entity * entity8 = EntityManager::Instance()->CreateEntity();
	entity8->AddComponent(VisibilityAABBoxComponent::Get());
	entity8->AddComponent(DrawMeshComponent::Get());;

	Entity * entity9 = EntityManager::Instance()->CreateEntity();
	entity9->AddComponent(VisibilityAABBoxComponent::Get());
	entity9->AddComponent(DrawMeshComponent::Get());

	Entity * entity10 = EntityManager::Instance()->CreateEntity();
	entity10->AddComponent(VisibilityAABBoxComponent::Get());
	entity10->AddComponent(DrawMeshComponent::Get());

	Entity * entity11 = EntityManager::Instance()->CreateEntity();
	entity11->AddComponent(VisibilityAABBoxComponent::Get());
	entity11->AddComponent(DrawMeshComponent::Get());

	Entity * entity12 = EntityManager::Instance()->CreateEntity();
	entity12->AddComponent(VisibilityAABBoxComponent::Get());
	entity12->AddComponent(DrawMeshComponent::Get());

	Entity * entity13 = EntityManager::Instance()->CreateEntity();
	entity13->AddComponent(VisibilityAABBoxComponent::Get());
	entity13->AddComponent(DrawMeshComponent::Get());

	Entity * entity14 = EntityManager::Instance()->CreateEntity();
	entity14->AddComponent(VisibilityAABBoxComponent::Get());
	entity14->AddComponent(DrawMeshComponent::Get());

	Entity * entity15 = EntityManager::Instance()->CreateEntity();
	entity15->AddComponent(VisibilityAABBoxComponent::Get());
	entity15->AddComponent(DrawMeshComponent::Get());

	manager->Flush();

	entity6->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity6->SetData("meshVisibilityFlag", (uint32)0);
	entity7->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity7->SetData("meshVisibilityFlag", (uint32)0);
	entity8->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity8->SetData("meshVisibilityFlag", (uint32)0);
	entity9->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity9->SetData("meshVisibilityFlag", (uint32)0);
	entity10->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity10->SetData("meshVisibilityFlag", (uint32)0);
	entity11->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(0.f, 1.f, 1.f)));
	entity11->SetData("meshVisibilityFlag", (uint32)0);
	entity12->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity12->SetData("meshVisibilityFlag", (uint32)0);
	entity13->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity13->SetData("meshVisibilityFlag", (uint32)0);
	entity14->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(0.f, 1.f, 1.f)));
	entity14->SetData("meshVisibilityFlag", (uint32)0);
	entity15->SetData("meshAABox", AABBox3(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f)));
	entity15->SetData("meshVisibilityFlag", (uint32)0);


	visibilityAABBoxSystem.Run();
	visibilityBSphereSystem.Run();
	drawSystem.Run();

	int32 expectedResultsResize[16] = {0,1,0,0,1,1,1,1,1,1,1,0,1,1,0,1};
	for(int32 i = 0; i < 16; ++i)
	{
		DVASSERT(testResults[i] == expectedResultsResize[i]);
	}

	manager->Dump();
    
    SafeRelease(manager);
}
