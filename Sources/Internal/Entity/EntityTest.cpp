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
    
    void Run()
    {
		Camera * camera = sceneCameraPools->at(0)->Get(0); 
        Frustum * frustum = camera->GetFrustum();
		
		int32 elementCount = meshAABBoxPool->GetCount();
		for (int k = 0; k < elementCount; ++k)
		{
			AABBox3 * bbox = meshAABBoxPool->GetPtr(k);
			uint32 * flagResult = meshVisibilityFlagPool->GetPtr(k);
			*flagResult = (frustum->IsInside(*bbox) == true);
		}
    }
    TemplatePool<void*> * meshPtrPool;
	TemplatePool<AABBox3> * meshAABBoxPool;
	TemplatePool<uint32> * meshVisibilityFlagPool;
	Vector<TemplatePool<Camera*> * > * sceneCameraPools;
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
    void Create()
    {
        //family = manager->GetFamily(VisibilitySphereComponent::Get());    
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
        
//        uint32 count = family->GetSize();
//        Sphere * sphereHead = family->GetPtr<Sphere>("meshBSphere");
//        uint32 * flags = family->GetPtr<uint32>("meshVisibilityFlag");
//        
//        for (uint32 k = 0; k < count; ++k)
//        {
//            sphereHead[k].radius = 1.0;
//            flags[k] = 1;
//        }
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


int main(int argc, const char ** argv)
{
    VisibilityAABBoxComponent::Register();
    VisibilitySphereComponent::Register();
    DrawMeshComponent::Register();
    
	//should be delayed (probably to the end of current frame)
	Entity * entity0 = EntityManager::Instance()->CreateEntity();
	entity0->AddComponent(VisibilityAABBoxComponent::Get());
	entity0->AddComponent(DrawMeshComponent::Get());
    //object0->Flush();
    
    entity0->SetData("meshAABox", AABBox3());
    entity0->SetData("visibilityFlag", 0);
    
    
    
}

