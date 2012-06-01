#include "EntityManager.h"

using namespace DAVA;


class VisibilityAABBoxComponent : public Component
{
public:
    void Create()
    {   
        meshPtr = CreatePool(StaticMesh*, "mesh"); 
        meshAABoxPool = CreatePool(AABBox, "meshAABox");
        meshVisibilityFlag = CreatePool(bool, "meshVisibilityFlag");
		sceneCameraPool = LinkToAllPools(Camera, "sceneCamera");
    }
    
    void Run()
    {
		Camera * camera = sceneCameraPool.Get(0); 
		
		int32 elementCount = meshAABBoxPool.length;
		for (int k = 0; k < elementCount; ++k)
		{
			AABBox * bbox = meshAABBoxPool->GetPtr(k);
			bool * flagResult = meshVisibilityFlagPool->GetPtr(k);
			*flagResult = (camera->IsInside(*bbox) == true);
		}
    }
	TemplatePool<AABBox*> meshAABoxPool;
	TemplatePool<bool*> meshVisibilityFlagPool;
	TemplatePool<Camera*> sceneCameraPool;
};

class VisibilitySphereComponent : public Component
{
public:
    void Create()
    {   
        meshPtr = CreatePool(StaticMesh*, "mesh"); 
        meshSpherePool = CreatePool(Sphere, "meshSphere");
        meshVisibilityFlag = CreatePool(bool, "meshVisibilityFlag");
		sceneCameraPool = LinkToAllPools(Camera, "sceneCamera");
    }
    
    void Run()
    {
		Camera * camera = sceneCameraPool.Get(0); 
		int32 elementCount = meshSpherePool.length;
		for (int k = 0; k < elementCount; ++k)
		{
			Sphere * sphere = meshSpherePool->GetPtr(k);
			bool * flagResult = meshVisibilityFlagPool->GetPtr(k);
			*flagResult = (camera->IsInside(*sphere) == true);
		}
    }
	TemplatePool<Sphere*> meshSpherePool;
	TemplatePool<bool*> meshVisibilityFlagPool;
	TemplatePool<Camera*> sceneCameraPool;
};

class DrawMeshComponent : public Component
{
public:
    void Create()
    {
		allMeshes = LinkToAllPools(Mesh*, "mesh");
        allMeshVisibilityPools = LinkToAllPools(bool, "meshVisibilityFlag");   
    }
    
    void Run()
    {
		for (int32 poolIndex = 0; poolIndex < allMeshVisibilityPools.GetPoolCount(); ++poolIndex)
		{
			int32 poolMeshCount = allMeshVisibilityPools.Get
			for (int32 visibilityIndex = 0; visibilityIndex < pool)
			
		}
    }

	Link<Pool*> allMeshVisibilityPools;
}



// declare component

ComponentDeclaration VisibilityAABBoxComponent
{
    bool, "visibilityFlag",
    AABBox3, "meshAABBox"
}

ComponentDeclaration VisibilityBSphereComponent
{
    bool, "visibilityFlag",
    Sphere, "meshSphere",
}

ComponentDeclaration DrawMeshComponent
{
    bool, "visibilityFlag",
    void*, "meshPtr",
    void*, "materialPtr",
}


int main(int argc, const char ** argv)
{
	//should be delayed (probably to the end of current frame)
	Entity * stone = EntityManager::Instance()->CreateEntity();
	stone->AddComponent("VisibilityAABBoxComponent");
	stone->AddComponent("DrawMeshComponent");
}