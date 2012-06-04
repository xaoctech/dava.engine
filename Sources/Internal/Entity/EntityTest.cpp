#include "Base/BaseMath.h"
#include "Entity/Entity.h"
#include "Entity/EntityManager.h"
#include "Entity/Component.h"
#include "Entity/ComponentDataMapper.h"
#include "Scene3D/Camera.h"

using namespace DAVA;


class VisibilityAABBoxComponent : public Component
{
public:

    static void Register()
    {   
        VisibilityAABBoxComponent * component = new VisibilityAABBoxComponent();
        RegisterComponent("VisibilityAABBoxComponent", component);
        component->Create();
    }
    
    void Create()
    {   
        RegisterData<AABBox3>("meshAABox");
        RegisterData<uint32>("meshVisibilityFlag");
    }
};

class VisibilityAABBoxSystem
{
public:
	EntityManager * manager;

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

class VisibilitySphereComponent : public Component
{
public:
    static void Register()
    {   
        VisibilitySphereComponent * component = new VisibilitySphereComponent();
        RegisterComponent("VisibilitySphereComponent", component);
        component->Create();
    }
    
    void Create()
    {   
        RegisterData<Sphere>("meshBSphere");
        RegisterData<uint32>("meshVisibilityFlag");
		//sceneCameraPools = LinkToAllPools((Camera*)(0), "sceneCamera");
    }
};

class VisibilitySphereSystem 
{
public:
    EntityManager * manager;
    EntityFamily * family;
    //ComponentDataMapper<Sphere> * sphereData;
    //ComponentDataMapper<uint32> * visibilityFlag;
    
    void Create()
    {
        family = manager->GetFamily(VisibilitySphereComponent::Get());    
    }
    
    void Run()
    {
		/*Camera * camera = sceneCameraPools->at(0)->Get(0); 
        Frustum * frustum = camera->GetFrustum();
		
		int32 elementCount = meshSpherePool->GetCount();
		for (int k = 0; k < elementCount; ++k)
		{
			Sphere * sphere = meshSpherePool->GetPtr(k);
			uint32 * flagResult = meshVisibilityFlagPool->GetPtr(k);
			*flagResult = 0; // No sphere check function: (frustum->IsInside(*sphere) == true);
		}*/
        //for (
        
        uint32 count = family->GetSize();
        Sphere * sphereHead = family->GetPtr<Sphere>("meshBSphere");
        uint32 * flags = family->GetPtr<uint32>("meshVisibilityFlag");
        
        for (uint32 k = 0; k < count; ++k)
        {
            sphereHead[k].radius = 1.0;
            flags[k] = 1;
        }
    }
//    TemplatePool<void*> * meshPtrPool;
//	TemplatePool<Sphere> * meshSpherePool;
//	TemplatePool<uint32> * meshVisibilityFlagPool;
//	Vector<TemplatePool<Camera*> * > * sceneCameraPools;
};

class DrawMeshComponent : public Component
{
public:
    static void Register()
    {   
        DrawMeshComponent * component = new DrawMeshComponent();
        RegisterComponent("DrawMeshComponent", component);
        component->Create();
    }
    
    void Create()
    {   
        RegisterData<void*>("mesh");
        RegisterData<uint32>("meshVisibilityFlag");
        RegisterData<Matrix4>("worldTransform");
    }
};



class DrawMeshSystem
{
public:
	EntityManager * manager;
    
    void Run()
    {
		TemplatePool<uint32> * visibilityFlags = manager->GetLinkedTemplatePools<uint32>("meshVisibilityFlag");
		while(visibilityFlags)
		{
			int32 count = visibilityFlags->GetCount();
			uint32 * flag = visibilityFlags->GetPtr(0);
			for(int32 i = 0; i < count; ++i)
			{
				Logger::Debug("visible %d", *flag);

				flag++;
			}

			visibilityFlags = visibilityFlags->next;
		}
    }
    
};



// declare component

//ComponentDeclaration VisibilityAABBoxComponent
//{
//    bool, "visibilityFlag",
//    AABBox3, "meshAABBox"
//}
//
//ComponentDeclaration VisibilityBSphereComponent
//{
//    bool, "visibilityFlag",
//    Sphere, "meshSphere",
//}
//
//ComponentDeclaration DrawMeshComponent
//{
//    bool, "visibilityFlag",
//    void*, "meshPtr",
//    void*, "materialPtr",
//}


void EntityTest()
{
    VisibilityAABBoxComponent::Register();
    VisibilitySphereComponent::Register();
    DrawMeshComponent::Register();

	EntityManager * manager = new EntityManager();

	VisibilityAABBoxSystem visibilityAABBoxSystem;
	visibilityAABBoxSystem.manager = manager;

	DrawMeshSystem drawSystem;
	drawSystem.manager = manager;

	Entity * entity0 = EntityManager::Instance()->CreateEntity();
	entity0->AddComponent(VisibilityAABBoxComponent::Get());
	entity0->AddComponent(DrawMeshComponent::Get());

    entity0->SetData("meshAABox", AABBox3());
    entity0->SetData("visibilityFlag", 0);
    
    SafeRelease(manager);
}
