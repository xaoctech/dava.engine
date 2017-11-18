#include "Concurrency/SyncBarrier.h"
#include "Concurrency/Thread.h"
#include "Engine/Engine.h"
#include "Logger/Logger.h"
#include "Time/SystemTimer.h"
#include "UnitTests/UnitTests.h"
#include "Utils/Random.h"

#include <mutex>

using namespace DAVA;

DAVA_TESTCLASS (SyncBarrierTest)
{
    static const int NTHREADS = 10;
    SyncBarrier barrier;
    Vector<Thread*> threads;
    std::atomic<int> counter1;
    std::atomic<int> counter2;
    int64 testStartTime = 0;
    bool testComplete = false;
    std::once_flag onceFlag;

    SyncBarrierTest()
        : barrier(NTHREADS)
    {
    }

    bool TestComplete(const String&)const override
    {
        if (testComplete)
        {
            for (Thread* t : threads)
            {
                t->Release();
            }
        }
        else
        {
            int64 testDuration = SystemTimer::GetMs() - testStartTime;
            if (testDuration > 10000)
            {
                TEST_VERIFY_WITH_MESSAGE(false, "SyncBarrierTest runs too long, maybe it hangs");
                return true;
            }
        }
        return testComplete;
    }

    DAVA_TEST (Test)
    {
        for (int i = 0; i < NTHREADS; ++i)
        {
            Thread* t = Thread::Create([this, i]() { ThreadFunc(i); });
            t->Start();
            threads.push_back(t);
        }
        testStartTime = SystemTimer::GetMs();
    }

    void ThreadFunc(int n)
    {
        for (int i = 0; i < 10; ++i)
        {
            counter1 = 0;
            counter2 = 0;
            barrier.Wait();

            counter1 += 1;
            Thread::Sleep((n % 4) * 100);
            barrier.Wait();
            TEST_VERIFY(counter1 == NTHREADS);

            counter2 += 1;
            barrier.Wait();
            TEST_VERIFY(counter2 == NTHREADS);

            counter1 = 0;
            barrier.Wait();
            TEST_VERIFY(counter1 == 0);

            barrier.Wait();
        }

        std::call_once(onceFlag, &RunOnMainThreadAsync, [this]() { testComplete = true; });
    }
};
