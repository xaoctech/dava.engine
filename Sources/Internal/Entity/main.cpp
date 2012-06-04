#include "Base/BaseMath.h"
#include "Entity/EntityManager.h"
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
		sceneCameraPools = LinkToAllPools((Camera*)(0), "sceneCamera");
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
		sceneCameraPools = LinkToAllPools((Camera*)(0), "sceneCamera");
    }

};

class VisibilitySphereSystem 
{
public:
    ComponentDataMapper<Sphere> * sphereData;
    ComponentDataMapper<uint32> * visibilityFlag;
    
    void Create()
    {
        
    }
    
    void Run()
    {
		Camera * camera = sceneCameraPools->at(0)->Get(0); 
        Frustum * frustum = camera->GetFrustum();
		
		int32 elementCount = meshSpherePool->GetCount();
		for (int k = 0; k < elementCount; ++k)
		{
			Sphere * sphere = meshSpherePool->GetPtr(k);
			uint32 * flagResult = meshVisibilityFlagPool->GetPtr(k);
			*flagResult = 0; // No sphere check function: (frustum->IsInside(*sphere) == true);
		}
    }
    TemplatePool<void*> * meshPtrPool;
	TemplatePool<Sphere> * meshSpherePool;
	TemplatePool<uint32> * meshVisibilityFlagPool;
	Vector<TemplatePool<Camera*> * > * sceneCameraPools;
};

class DrawMeshComponent : public Component
{
public:
    void Create()
    {
		allMeshesPools = LinkToAllPools((void*)(0), "mesh");
        allMeshVisibilityFlagPools = LinkToAllPools(uint32(), "meshVisibilityFlag");   
    }
    
    void Run()
    {
		for (int32 poolIndex = 0; poolIndex < allMeshVisibilityFlagPools->size(); ++poolIndex)
		{
			int32 poolMeshCount = allMeshVisibilityFlagPools->at(poolIndex)->GetCount();
			for (int32 visibilityIndex = 0; visibilityIndex < poolMeshCount; ++visibilityIndex)
            {
                /*
                    Мысль такая, что если visibilityIndex всегда связан с Mesh-ем, то его можно положить прямо в 
                    Mesh, ведь по сути, мы всегда когда рисуем меши, перед отрисовкой меша, будем читать 
                    visibilityIndex, если он будет лежать перед мешем то в теории, его чтение может подогреть кэш
                 */
                /*
                    Мысль 2: Надо написать вариант, когда в начале получаются данные. Указатели на visibility, mesh
                    и в внутреннем цикле ходим уже только по указателям. 
                 */
                // retrieve visibility index
                // get mesh
                // draw mesh if visibility index is good
                
            }   
		}
    }

	Vector<TemplatePool<void*>*> * allMeshesPools;
    Vector<TemplatePool<uint32> *> * allMeshVisibilityFlagPools;
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
    
	//should be delayed (probably to the end of current frame)
	Entity * object0 = EntityManager::Instance()->CreateEntity();
	object0->AddComponent(VisibilityAABBoxComponent::Get());
	object0->AddComponent(DrawMeshComponent::Get());
    //object0->Flush();
    
    
    
    
    
}
