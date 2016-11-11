#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"
#include "Utils/Random.h"
#include <functional>

using namespace DAVA;

DAVA_TESTCLASS (SceneIdTest)
{
    DAVA_TEST (TestFunc)
    {
        const uint32 nodesToAdd = 100;
        FilePath testScenePath1 = "~doc://test1_scene_id.sc2";
        FilePath testScenePath2 = "~doc://test2_scene_id.sc2";

        Scene* scene1 = new Scene();
        Entity* fake1_1 = CreateFakeEntity();
        Entity* fake1_2 = CreateFakeEntity();
        Entity* fake1_3 = CreateFakeEntity();

        for (auto i = 0; i < nodesToAdd; ++i)
        {
            Entity* __fake1 = CreateFakeEntity();
            Entity* __fake2 = CreateFakeEntity();
            Entity* __fake3 = CreateFakeEntity();
            fake1_1->AddNode(__fake1);
            fake1_2->AddNode(__fake2);
            fake1_3->AddNode(__fake3);
            __fake1->Release();
            __fake2->Release();
            __fake3->Release();
        }

        scene1->AddNode(fake1_1);
        scene1->AddNode(fake1_2);

        // save/load scene test
        scene1->SaveScene(testScenePath1);

        Scene* scene2 = new Scene();
        scene2->LoadScene(testScenePath1);

        TEST_VERIFY(CompareScene(scene1, scene2));

        // remove/add entity test
        uint32 entityToExtractIndex = Random::Instance()->Rand(nodesToAdd - 1);
        Entity* entityToExtract = scene1->children[0];
        Entity* entityBefore = scene1->children[1];

        SafeRetain(entityToExtract);
        scene1->RemoveNode(entityToExtract);
        scene1->InsertBeforeNode(entityToExtract, entityBefore);
        SafeRelease(entityToExtract);

        TEST_VERIFY(CompareScene(scene1, scene2));

        SafeRelease(fake1_1);
        SafeRelease(fake1_2);
        SafeRelease(fake1_3);
        SafeRelease(scene1);
        SafeRelease(scene2);
    }

    Entity* CreateFakeEntity()
    {
        char tmp[16];
        Entity* entity = new Entity();

        uint32 index = Random::Instance()->Rand();
        sprintf(tmp, "%u", index);

        entity->SetName(tmp);
        entity->AddComponent(new UserComponent());

        return entity;
    }

    bool CompareScene(Scene * src, Scene * dst)
    {
        std::function<bool(Entity * entity1, Entity * entity2)> compareID = [&compareID](Entity* entity1, Entity* entity2) -> bool
        {
            bool ret = true;

            if (entity1->children.size() == entity2->children.size() &&
                ((entity1->GetName() == entity2->GetName()) ||
                 (entity1->GetScene() == entity1 && entity2->GetScene() == entity2)))
            {
                for (size_t i = 0; i < entity1->children.size(); ++i)
                {
                    if (!compareID(entity1->children[i], entity2->children[i]))
                    {
                        ret = false;
                        break;
                    }
                }
            }
            else
            {
                ret = false;
            }

            return ret;
        };

        return compareID(src, dst);
    }
}
;
