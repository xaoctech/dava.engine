#include <UnitTests/UnitTests.h>

#include <Asset/AssetListener.h>
#include <Asset/AssetManager.h>
#include <Base/BaseTypes.h>
#include <Base/String.h>
#include <Debug/DebuggerDetection.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <Render/3D/Geometry.h>
#include <Render/Material/Material.h>
#include <Render/Material/NMaterial.h>
#include <Time/SystemTimer.h>

namespace AssetManagerTestsDetail
{
using namespace DAVA;

FilePath workDir;
Array<FilePath, 5> material;
FilePath geomPath;

struct ListenerResult
{
    enum eEvent
    {
        Loaded,
        Reloaded,
        Error,
        ErrorReloaded,
        Unloaded
    };

    const AssetBase* asset;
    eEvent e;
};

class TestListener : public AssetListener
{
public:
    void OnAssetLoaded(const Asset<AssetBase>& asset, bool reloaded)
    {
        ListenerResult r;
        r.asset = asset.get();
        r.e = reloaded == false ? ListenerResult::Loaded : ListenerResult::Reloaded;

        results.push_back(r);
    }

    void OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const DAVA::String& msg) override
    {
        ListenerResult r;
        r.asset = asset.get();
        r.e = reloaded == false ? ListenerResult::Error : ListenerResult::ErrorReloaded;

        results.push_back(r);
    }

    void OnAssetUnloaded(const AssetBase* asset) override
    {
        ListenerResult r;
        r.asset = asset;
        r.e = ListenerResult::Unloaded;

        results.push_back(r);
    }

    Vector<ListenerResult> results;
};

class BaseTest
{
public:
    virtual ~BaseTest()
    {
        AssetManager* assetManager = GetEngineContext()->assetManager;
        std::for_each(listeners.begin(), listeners.end(), [this, assetManager](AssetListener* listener) {
            assetManager->UnregisterListener(listener);
        });
    }
    void Run()
    {
        startMs = SystemTimer::GetMs();
        RunImpl();
    }

    bool IsCompleted()
    {
        if (IsCompletedImpl() == true)
        {
            return true;
        }

        if (DAVA::IsDebuggerPresent() == false)
        {
            bool outOfTime = (SystemTimer::GetMs() - startMs) > 1000;
            TEST_VERIFY(outOfTime == false);
            return outOfTime;
        }

        return false;
    }

protected:
    void RegisterListener(AssetListener* listener)
    {
        listeners.push_back(listener);
    }

    virtual void RunImpl() = 0;
    virtual bool IsCompletedImpl()
    {
        if (isFirstCompleteCall == true)
        {
            isFirstCompleteCall = false;
            return false;
        }

        return true;
    }

private:
    uint64 startMs = 0;
    bool isFirstCompleteCall = true;
    Vector<AssetListener*> listeners;
};

class CreateAssetT : public BaseTest
{
protected:
    void RunImpl() override
    {
        AssetManager* assetManager = GetEngineContext()->assetManager;

        for (int32 i = 0; i < material.size(); ++i)
        {
            Material::PathKey matKey(material[i]);
            Asset<Material> asset = assetManager->CreateAsset<Material>(matKey);
            ScopedPtr<NMaterial> nmat(new NMaterial());
            nmat->SetFXName(NMaterialName::VERTEXLIT_OPAQUE);
            nmat->SetMaterialName(FastName(Format("dummy_material_%d", i)));
            asset->SetMaterial(nmat);

            bool result = assetManager->SaveAsset(asset);
            TEST_VERIFY(result == true);
            TEST_VERIFY(asset->GetState() == AssetBase::LOADED);

            Asset<Material> lookedUpAsset = assetManager->FindAsset<Material>(matKey);
            TEST_VERIFY(lookedUpAsset == asset);

            TEST_VERIFY(GetEngineContext()->fileSystem->Exists(material[i]));
        }

        for (int32 i = 0; i < material.size(); ++i)
        {
            Material::PathKey matKey(material[i]);
            TEST_VERIFY(assetManager->FindAsset<Material>(matKey) == nullptr);
        }
    }
};

class SyncLoadMaterialT : public BaseTest
{
    TestListener listener;
    Material* assetRawPtr = nullptr;

protected:
    void RunImpl() override
    {
        RegisterListener(&listener);
        Material::PathKey key(material[0]);
        AssetManager* assetManager = GetEngineContext()->assetManager;
        {
            Asset<Material> matAsset = assetManager->GetAsset<Material>(key, AssetManager::SYNC, &listener);
            assetRawPtr = matAsset.get();
            TEST_VERIFY(matAsset != nullptr);
            TEST_VERIFY(matAsset->GetState() == AssetBase::LOADED);
            TEST_VERIFY(matAsset == assetManager->FindAsset<Material>(key));

            TEST_VERIFY(listener.results.size() == 1);
            TEST_VERIFY(listener.results[0].e == ListenerResult::Loaded);
            TEST_VERIFY(listener.results[0].asset == matAsset.get());
            listener.results.clear();
        }

        // error notify test
        {
            Asset<Material> matAsset = assetManager->GetAsset<Material>(Material::PathKey(workDir + "failed.mat"), AssetManager::SYNC, &listener);
            TEST_VERIFY(matAsset != nullptr);
            TEST_VERIFY(matAsset->GetState() == AssetBase::ERROR);
            TEST_VERIFY(listener.results.size() == 1);
            TEST_VERIFY(listener.results[0].e == ListenerResult::Error);
            TEST_VERIFY(listener.results[0].asset == matAsset.get());
            listener.results.clear();
        }
    }
};

class LoadedAssetListenerT : public BaseTest
{
    TestListener listener1;
    TestListener listener2;

protected:
    void RunImpl() override
    {
        RegisterListener(&listener1);
        RegisterListener(&listener2);

        Material::PathKey key(material[0]);
        AssetManager* assetManager = GetEngineContext()->assetManager;

        {
            Asset<Material> matAsset1 = assetManager->GetAsset<Material>(key, AssetManager::SYNC, &listener1);
            TEST_VERIFY(matAsset1->GetState() == AssetBase::LOADED);
            TEST_VERIFY(listener1.results.size() == 1);
            TEST_VERIFY(listener1.results[0].asset == matAsset1.get());
            TEST_VERIFY(listener1.results[0].e == ListenerResult::Loaded);
            listener1.results.clear();

            Asset<Material> matAsset2 = assetManager->GetAsset<Material>(key, AssetManager::SYNC, &listener2);
            TEST_VERIFY(matAsset2->GetState() == AssetBase::LOADED);
            TEST_VERIFY(listener2.results.size() == 1);
            TEST_VERIFY(listener2.results[0].asset == matAsset2.get());
            TEST_VERIFY(listener2.results[0].e == ListenerResult::Loaded);
            listener2.results.clear();
        }
    }
};

class TypeListenerT : public BaseTest
{
    TestListener typeListener;
    TestListener instanceListener;
    Material* assetRawPtr = nullptr;

protected:
    void RunImpl() override
    {
        RegisterListener(&typeListener);
        RegisterListener(&instanceListener);
        Material::PathKey key(material[0]);
        AssetManager* assetManager = GetEngineContext()->assetManager;
        assetManager->RegisterListener(&typeListener, Type::Instance<Material>());

        {
            Asset<Material> matAsset = assetManager->GetAsset<Material>(key, AssetManager::SYNC, &instanceListener);
            assetRawPtr = matAsset.get();
            TEST_VERIFY(matAsset->GetState() == AssetBase::LOADED);
            TEST_VERIFY(typeListener.results.size() == 1);
            TEST_VERIFY(typeListener.results[0].asset == matAsset.get());
            TEST_VERIFY(typeListener.results[0].e == ListenerResult::Loaded);

            TEST_VERIFY(instanceListener.results.size() == 1);
            TEST_VERIFY(instanceListener.results[0].asset == matAsset.get());
            TEST_VERIFY(instanceListener.results[0].e == ListenerResult::Loaded);
        }

        typeListener.results.clear();
        instanceListener.results.clear();

        {
            // Asset has not deleted yet, so typeListener would not been notified
            Asset<Material> matAsset = assetManager->GetAsset<Material>(key, AssetManager::SYNC);
            TEST_VERIFY(typeListener.results.size() == 0);
            TEST_VERIFY(instanceListener.results.size() == 0);
        }
    }

    bool IsCompletedImpl() override
    {
        if (typeListener.results.size() == 1)
        {
            TEST_VERIFY(typeListener.results[0].asset == assetRawPtr);
            TEST_VERIFY(typeListener.results[0].e == ListenerResult::Unloaded);
            TEST_VERIFY(instanceListener.results.empty() == true);
            return true;
        }

        return false;
    }
};

class CommonListenerT : public BaseTest
{
    TestListener typeListener;
    TestListener instanceListener;
    TestListener commonListener;

    Geometry* geomRawPtr = nullptr;
    Material* matRawPtr = nullptr;

protected:
    void RunImpl() override
    {
        RegisterListener(&typeListener);
        RegisterListener(&instanceListener);
        RegisterListener(&commonListener);
        Geometry::PathKey geoKey(geomPath);
        Material::PathKey matKey(material[0]);
        AssetManager* assetManager = GetEngineContext()->assetManager;
        assetManager->RegisterListener(&typeListener, Type::Instance<Material>());
        assetManager->RegisterListener(&commonListener, nullptr);

        {
            Asset<Geometry> geomAsset = assetManager->CreateAsset<Geometry>(geoKey);
            geomRawPtr = geomAsset.get();
            assetManager->SaveAsset(geomAsset);

            TEST_VERIFY(typeListener.results.empty() == true);
            TEST_VERIFY(instanceListener.results.empty() == true);
            TEST_VERIFY(commonListener.results.empty() == false);
        }
        {
            Asset<Material> matAsset = assetManager->GetAsset<Material>(matKey, AssetManager::SYNC);
            matRawPtr = matAsset.get();
        }
        {
            // geometry asset has not been deleted yet, so GetAsset return same reference
            // it means that common and type listener will be not notified
            Asset<Geometry> geoAsset = assetManager->GetAsset<Geometry>(geoKey, AssetManager::SYNC, &instanceListener);
            TEST_VERIFY(geomRawPtr == geoAsset.get());
        }

        TEST_VERIFY(commonListener.results.size() == 2);
        TEST_VERIFY(commonListener.results[0].asset == geomRawPtr);
        TEST_VERIFY(commonListener.results[0].e == ListenerResult::Loaded);
        TEST_VERIFY(commonListener.results[1].asset == matRawPtr);
        TEST_VERIFY(commonListener.results[1].e == ListenerResult::Loaded);
        commonListener.results.clear();

        TEST_VERIFY(instanceListener.results.size() == 1);
        TEST_VERIFY(instanceListener.results[0].asset == geomRawPtr);
        TEST_VERIFY(instanceListener.results[0].e == ListenerResult::Loaded);
        instanceListener.results.clear();

        TEST_VERIFY(typeListener.results.size() == 1);
        TEST_VERIFY(typeListener.results[0].asset == matRawPtr);
        TEST_VERIFY(typeListener.results[0].e == ListenerResult::Loaded);
        typeListener.results.clear();
    }

    bool IsCompletedImpl() override
    {
        if (commonListener.results.empty() == false)
        {
            TEST_VERIFY(commonListener.results[0].asset == geomRawPtr);
            TEST_VERIFY(commonListener.results[0].e == ListenerResult::Unloaded);
            TEST_VERIFY(commonListener.results[1].asset == matRawPtr);
            TEST_VERIFY(commonListener.results[1].e == ListenerResult::Unloaded);
            TEST_VERIFY(typeListener.results.size() == 1);
            TEST_VERIFY(typeListener.results[0].asset == matRawPtr);
            TEST_VERIFY(typeListener.results[0].e == ListenerResult::Unloaded);
            TEST_VERIFY(instanceListener.results.empty() == true);

            return true;
        }

        return false;
    }
};

class AsyncMaterialLoadT : public BaseTest
{
    Asset<Material> asset;
    TestListener instanceListener;

public:
    void RunImpl() override
    {
        RegisterListener(&instanceListener);
        Material::PathKey matKey(material[0]);
        AssetManager* assetManager = GetEngineContext()->assetManager;

        asset = assetManager->GetAsset<Material>(matKey, AssetManager::ASYNC, &instanceListener);
    }

    bool IsCompletedImpl() override
    {
        if (instanceListener.results.empty() == true)
        {
            return false;
        }

        TEST_VERIFY(asset->GetState() == AssetBase::LOADED);
        TEST_VERIFY(instanceListener.results[0].e == ListenerResult::Loaded);
        TEST_VERIFY(instanceListener.results[0].asset == asset.get());

        return true;
    }
};

class SyncAfterAsyncLoadT : public BaseTest
{
public:
    void RunImpl() override
    {
        Material::PathKey matKey(material[0]);
        AssetManager* assetManager = GetEngineContext()->assetManager;

        Asset<Material> asset = assetManager->GetAsset<Material>(matKey, AssetManager::ASYNC);
        TEST_VERIFY(asset->GetState() == AssetBase::QUEUED);

        Asset<Material> syncAsset = assetManager->GetAsset<Material>(matKey, AssetManager::SYNC);
        TEST_VERIFY(asset->GetState() == AssetBase::LOADED);
        TEST_VERIFY(syncAsset->GetState() == AssetBase::LOADED);
        TEST_VERIFY(asset == syncAsset);
    }
};

class SyncAfterAsyncLoadWithRemoveFromQueueT : public BaseTest
{
    Vector<Asset<Material>> assets;

public:
    void RunImpl() override
    {
        AssetManager* assetManager = GetEngineContext()->assetManager;

        assets.resize(material.size());
        for (int32 i = 0; i < material.size() - 1; ++i)
        {
            Material::PathKey matKey(material[i]);
            assets[i] = assetManager->GetAsset<Material>(matKey, AssetManager::ASYNC);
            TEST_VERIFY(assets[i]->GetState() == AssetBase::QUEUED);
        }

        Material::PathKey matKey(material[material.size() - 1]);
        assets[material.size() - 1] = assetManager->GetAsset<Material>(matKey, AssetManager::ASYNC);
        TEST_VERIFY(assets[material.size() - 1]->GetState() == AssetBase::QUEUED);

        Asset<Material> syncAsset = assetManager->GetAsset<Material>(matKey, AssetManager::SYNC);
        TEST_VERIFY(assets[material.size() - 1]->GetState() == AssetBase::LOADED);
        TEST_VERIFY(syncAsset->GetState() == AssetBase::LOADED);
        TEST_VERIFY(assets[material.size() - 1] == syncAsset);
    }

    bool IsCompletedImpl() override
    {
        for (Asset<Material> m : assets)
        {
            TEST_VERIFY(m->GetState() != AssetBase::ERROR);
            if (m->GetState() != AssetBase::LOADED)
            {
                return false;
            }
        }

        return true;
    }
};

class CancelAsyncLoadingT : public BaseTest
{
    TestListener typeListener;

public:
    void RunImpl() override
    {
        RegisterListener(&typeListener);
        AssetManager* assetManager = GetEngineContext()->assetManager;
        assetManager->RegisterListener(&typeListener, nullptr);

        for (int32 i = 0; i < material.size(); ++i)
        {
            Material::PathKey matKey(material[i]);
            Asset<AssetBase> asset = assetManager->GetAsset(matKey, AssetManager::ASYNC);
            TEST_VERIFY(asset->GetState() == AssetBase::QUEUED);
        }
    }

    bool IsCompletedImpl() override
    {
        if (typeListener.results.size() == material.size())
        {
            for (size_t i = 0; i < typeListener.results.size(); ++i)
            {
                TEST_VERIFY(typeListener.results[i].e == ListenerResult::Unloaded);
            }

            return true;
        }

        return false;
    }
};
} // namespace AssetManagerTestsDetail

DAVA_TESTCLASS (AssetManagerTests)
{
    AssetManagerTestsDetail::BaseTest* test = nullptr;

    AssetManagerTests()
    {
        using namespace DAVA;
        using namespace AssetManagerTestsDetail;

        FileSystem* fs = GetEngineContext()->fileSystem;
        workDir = fs->GetTempDirectoryPath();
        workDir = workDir.MakeDirectoryPathname() + "AssetManagerTests";
        workDir.MakeDirectoryPathname();

        fs->DeleteDirectory(workDir);
        fs->CreateDirectory(workDir, true);

        for (int32 i = 0; i < material.size(); ++i)
        {
            material[i] = workDir + Format("%d.mat", i);
        }

        geomPath = workDir + "geometry.geo";
    }

    ~AssetManagerTests()
    {
        using namespace DAVA;
        using namespace AssetManagerTestsDetail;

        FileSystem* fs = GetEngineContext()->fileSystem;
        fs->DeleteDirectory(workDir);
    }

    bool TestComplete(const DAVA::String& testName) const override
    {
        DVASSERT(test != nullptr);
        return test->IsCompleted();
    }

    void TearDown(const DAVA::String& testName) override
    {
        DVASSERT(test != nullptr);
        delete test;
        test = nullptr;
    }

    template <typename T>
    void RunTest()
    {
        test = new T();
        test->Run();
    }

    DAVA_TEST (CreateAssetTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<CreateAssetT>();
    }

    DAVA_TEST (SyncLoadMaterialTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<SyncLoadMaterialT>();
    }

    DAVA_TEST (LoadedAssetListenerTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<LoadedAssetListenerT>();
    }

    DAVA_TEST (TypeListenerTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<TypeListenerT>();
    }

    DAVA_TEST (CommonListenerTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<CommonListenerT>();
    }

    DAVA_TEST (AsyncMaterialLoadTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<AsyncMaterialLoadT>();
    }

    DAVA_TEST (SyncAfterAsyncLoadTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<SyncAfterAsyncLoadT>();
    }

    DAVA_TEST (SyncAfterAsyncLoadWithRemoveFromQueueTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<SyncAfterAsyncLoadWithRemoveFromQueueT>();
    }

    DAVA_TEST (CancelAsyncLoadingTest)
    {
        using namespace AssetManagerTestsDetail;
        RunTest<CancelAsyncLoadingT>();
    }
};
