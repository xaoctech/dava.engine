#ifndef __ENTITY_TEST_H__
#define __ENTITY_TEST_H__

#include "DAVAEngine.h"
using namespace DAVA;

#include "TestTemplate.h"

//DECLARE_COMPONENT(TestVisibilityAABBoxComponent);  
//
//void TestVisibilityAABBoxComponent::Register()
//{   
//	RegisterData<AABBox3>("meshAABox");
//	RegisterData<uint32>("meshVisibilityFlag");
//}
//
//DECLARE_COMPONENT(TestVisibilityBSphereComponent);  
//void TestVisibilityBSphereComponent::Register()
//{   
//	RegisterData<Sphere>("meshBSphere");
//	RegisterData<uint32>("meshVisibilityFlag");
//}
//
//DECLARE_COMPONENT(TestDrawMeshComponent);
//void TestDrawMeshComponent::Register()
//{
//	RegisterData<uint32>("meshVisibilityFlag");
//	RegisterData<Matrix4>("worldTransform");
//}

class EntityTest : public TestTemplate<EntityTest>
{
public:
	EntityTest();

	virtual void LoadResources();
	virtual void UnloadResources();

	void DummyComponents(PerfFuncData * data);

};


#endif // __SPRITETEST_H__
