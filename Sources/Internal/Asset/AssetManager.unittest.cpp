#include <UnitTests/UnitTests.h>

#include <Asset/AssetListener.h>
#include <Asset/AssetManager.h>
#include <Base/BaseTypes.h>
#include <Debug/DebuggerDetection.h>
#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/FileSystem.h>
#include <Render/3D/Geometry.h>
#include <Render/Material/Material.h>
#include <Render/Material/NMaterial.h>
#include <Scene3D/AssetLoaders/GeometryAssetLoader.h>
#include <Scene3D/AssetLoaders/MaterialAssetLoader.h>
#include <Time/SystemTimer.h>

namespace AssetManagerTestsDetail
{
using namespace DAVA;

FilePath workDir;
FilePath materialPath;
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
    void OnAssetLoaded(const Asset<AssetBase>& asset, bool reloaded) override
    {
        ListenerResult r;
        r.asset = asset.get();
        r.e = reloaded == false ? ListenerResult::Loaded : ListenerResult::Reloaded;

        results.push_back(r);
    }

    void OnAssetError(const Asset<AssetBase>& asset, bool reloaded, const String& msg) override
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
    ~BaseTest()
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
        MaterialAssetLoader::PathKey key(materialPath);
        AssetManager* assetManager = GetEngineContext()->assetManager;

        {
            Asset<Material> matAsset = assetManager->CreateAsset<Material>(key);
            TEST_VERIFY(matAsset->GetState() == AssetBase::EMPTY);

            ScopedPtr<NMaterial> nmat(new NMaterial());
            nmat->SetMaterialName(FastName("dummy"));
            matAsset->SetMaterial(nmat);
            bool result = assetManager->SaveAsset(matAsset);
            TEST_VERIFY(result == true);
            TEST_VERIFY(matAsset->GetState() == AssetBase::LOADED);

            Asset<Material> lookedUpAsset = assetManager->FindAsset<Material>(key);
            TEST_VERIFY(lookedUpAsset == matAsset);

            TEST_VERIFY(GetEngineContext()->fileSystem->Exists(materialPath));
        }

        TEST_VERIFY(assetManager->FindAsset<Material>(key) == nullptr);
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
        MaterialAssetLoader::PathKey key(materialPath);
        AssetManager* assetManager = GetEngineContext()->assetManager;
        {
            Asset<Material> matAsset = assetManager->GetAsset<Material>(key, false, &listener);
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
            Asset<Material> matAsset = assetManager->GetAsset<Material>(MaterialAssetLoader::PathKey(workDir + "1.mat"), false, &listener);
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

        MaterialAssetLoader::PathKey key(materialPath);
        AssetManager* assetManager = GetEngineContext()->assetManager;

        {
            Asset<Material> matAsset1 = assetManager->GetAsset<Material>(key, false, &listener1);
            TEST_VERIFY(matAsset1->GetState() == AssetBase::LOADED);
            TEST_VERIFY(listener1.results.size() == 1);
            TEST_VERIFY(listener1.results[0].asset == matAsset1.get());
            TEST_VERIFY(listener1.results[0].e == ListenerResult::Loaded);
            listener1.results.clear();

            Asset<Material> matAsset2 = assetManager->GetAsset<Material>(key, false, &listener2);
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
        MaterialAssetLoader::PathKey key(materialPath);
        AssetManager* assetManager = GetEngineContext()->assetManager;
        assetManager->RegisterListener(&typeListener, Type::Instance<Material>());

        {
            Asset<Material> matAsset = assetManager->GetAsset<Material>(key, false, &instanceListener);
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
            Asset<Material> matAsset = assetManager->GetAsset<Material>(key, false);
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
        GeometryAssetLoader::PathKey geoKey(geomPath);
        MaterialAssetLoader::PathKey matKey(materialPath);
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
            Asset<Material> matAsset = assetManager->GetAsset<Material>(matKey, false);
            matRawPtr = matAsset.get();
        }
        {
            // geometry asset has not been deleted yet, so GetAsset return same reference
            // it means that common and type listener will be not notified
            Asset<Geometry> geoAsset = assetManager->GetAsset<Geometry>(geoKey, false, &instanceListener);
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

        materialPath = workDir + "materialAsset.mat";
        geomPath = workDir + "geometry.geo";
    }

    ~AssetManagerTests()
    {
        using namespace DAVA;
        using namespace AssetManagerTestsDetail;

        FileSystem* fs = GetEngineContext()->fileSystem;
        fs->DeleteDirectory(workDir);
    }

    bool TestComplete(const String& testName) const override
    {
        DVASSERT(test != nullptr);
        return test->IsCompleted();
    }

    void TearDown(const String& testName) override
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
};