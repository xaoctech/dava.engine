#include "UnitTests/UnitTests.h"

#include "Base/HashVector.h"
#include "Scene3D/Entity.h"

using namespace DAVA;

DAVA_TESTCLASS (HashVectorTest)
{
    DAVA_TEST (SizeAddGet)
    {
        HashVector<Entity> c;

        TEST_VERIFY(c.GetSize() == 0);

        Entity* e = new Entity;
        SCOPE_EXIT
        {
            e->Release();
        };
        c.Add(e);
        TEST_VERIFY(c.GetSize() == 1);

        Entity* e2 = new Entity;
        SCOPE_EXIT
        {
            e2->Release();
        };
        c.Add(e2);
        TEST_VERIFY(c.GetSize() == 2);
    };

    DAVA_TEST (RemoveContains)
    {
        HashVector<Entity> c;

        Entity* e = new Entity;
        SCOPE_EXIT
        {
            e->Release();
        };
        c.Add(e);
        TEST_VERIFY(c.GetSize() == 1);

        c.Remove(e);
        TEST_VERIFY(c.GetSize() == 0);

        c.Add(e);

        Entity* e2 = new Entity;
        SCOPE_EXIT
        {
            e2->Release();
        };
        c.Add(e2);

        Entity* e3 = new Entity;
        SCOPE_EXIT
        {
            e3->Release();
        };
        c.Add(e3);

        TEST_VERIFY(c.GetSize() == 3);

        c.Remove(e2);
        TEST_VERIFY(c.GetSize() == 2);
        TEST_VERIFY(c.Contains(e) == true);
        TEST_VERIFY(c.Contains(e2) == false);
        TEST_VERIFY(c.Contains(e3) == true);

        c.Remove(e3);
        TEST_VERIFY(c.GetSize() == 1);
        TEST_VERIFY(c.Contains(e) == true);
        TEST_VERIFY(c.Contains(e2) == false);
        TEST_VERIFY(c.Contains(e3) == false);

        c.Remove(e);
        TEST_VERIFY(c.GetSize() == 0);
        TEST_VERIFY(c.Contains(e) == false);
        TEST_VERIFY(c.Contains(e2) == false);
        TEST_VERIFY(c.Contains(e3) == false);
    }

    DAVA_TEST (Objects)
    {
        HashVector<Entity> c;
        Entity* e = new Entity;
        SCOPE_EXIT
        {
            e->Release();
        };
        c.Add(e);
        Entity* e2 = new Entity;
        SCOPE_EXIT
        {
            e2->Release();
        };
        c.Add(e2);
        Entity* e3 = new Entity;
        SCOPE_EXIT
        {
            e3->Release();
        };
        c.Add(e3);

        auto& vec = c.GetObjects();
        TEST_VERIFY(vec[0] == e);
        TEST_VERIFY(vec[1] == e2);
        TEST_VERIFY(vec[2] == e3);
    }

    DAVA_TEST (ObjectsAtIndexOf)
    {
        HashVector<Entity> c;
        Entity* e = new Entity;
        SCOPE_EXIT
        {
            e->Release();
        };
        c.Add(e);
        Entity* e2 = new Entity;
        SCOPE_EXIT
        {
            e2->Release();
        };
        c.Add(e2);
        Entity* e3 = new Entity;
        SCOPE_EXIT
        {
            e3->Release();
        };
        c.Add(e3);

        TEST_VERIFY(c.GetObjectAt(0) == e);
        TEST_VERIFY(c.GetObjectAt(1) == e2);
        TEST_VERIFY(c.GetObjectAt(2) == e3);

        TEST_VERIFY(c.GetIndexOf(e) == 0);
        TEST_VERIFY(c.GetIndexOf(e2) == 1);
        TEST_VERIFY(c.GetIndexOf(e3) == 2);
    }

    DAVA_TEST (RangedFor)
    {
        HashVector<Entity> c;

        int32 executedCount = 0;
        for (Entity* e : c)
        {
            executedCount++;
        }
        TEST_VERIFY(executedCount == 0);

        Entity* e1 = new Entity;
        SCOPE_EXIT
        {
            e1->Release();
        };
        c.Add(e1);
        Entity* e2 = new Entity;
        SCOPE_EXIT
        {
            e2->Release();
        };
        c.Add(e2);
        Entity* e3 = new Entity;
        SCOPE_EXIT
        {
            e3->Release();
        };
        c.Add(e3);

        executedCount = 0;
        for (Entity* e : c)
        {
            executedCount++;
        }
        TEST_VERIFY(executedCount == 3);
    }

    DAVA_TEST (Clear)
    {
        HashVector<Entity> c;

        TEST_VERIFY(c.GetSize() == 0);

        Entity* e = new Entity;
        SCOPE_EXIT
        {
            e->Release();
        };
        c.Add(e);
        Entity* e2 = new Entity;
        SCOPE_EXIT
        {
            e2->Release();
        };
        c.Add(e2);
        Entity* e3 = new Entity;
        SCOPE_EXIT
        {
            e3->Release();
        };
        c.Add(e3);

        TEST_VERIFY(c.GetSize() != 0);

        c.Clear();
        TEST_VERIFY(c.GetSize() == 0);
    }
};