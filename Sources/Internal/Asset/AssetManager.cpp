#include "Asset/AssetBase.h"
#include "Asset/AssetManager.h"
#include "Job/JobManager.h"
#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
AssetManager::AssetManager()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(AssetBase);

    Engine* engine = Engine::Instance();
    engine->PrimaryWindow()->focusChanged.Connect(MakeFunction(this, &AssetManager::OnAppFocusChanged));

    FilePath dataFilePath("~res:/");
    FilePath dataSourceFilePath("~res:/../DataSource/");
    fileWatcher.AddWatch(dataFilePath.GetAbsolutePathname(), true);
    //fileWatcher.AddWatch("/Users/vitaliyborodovsky/Sources/Rendering", true);

    fileWatcher.onWatchersChanged.Connect(MakeFunction(this, &AssetManager::OnFWChanged));
    fileWatcher.StartUpdateThread();
}

AssetManager::~AssetManager()
{
    fileWatcher.FinishUpdateThread();
}

void AssetManager::ReloadAllChangedFiles()
{
    if (!changedFiles.empty())
    {
        Logger::Debug<AssetManager>("[AssetManager] Found Changed Files And Reload all of them");
        for (const auto& file : changedFiles)
        {
            OnAssetChanged(FilePath(file));
        }
        changedFiles.clear();
    }
}

void AssetManager::OnAppFocusChanged(Window*, bool visible)
{
    if (visible)
    {
        Logger::Debug<AssetManager>("[AssetManager] On App Focus Become Visible");
        ReloadAllChangedFiles();
    }
}

void AssetManager::OnAppResume()
{
    Logger::Debug<AssetManager>("[AssetManager] On App Resume");
    ReloadAllChangedFiles();
}

void AssetManager::OnFWChanged(FW::WatchID watchId, FW::String dir, FW::String filename, FW::Action action)
{
    Logger::Debug<AssetManager>("[AssetManager] %s changed action:%d", filename.c_str(), action);
    changedFiles.insert(filename);
}

void AssetManager::OnAssetChanged(const FilePath& filepath)
{
    Asset<AssetBase> asset = FindAsset<AssetBase>(filepath);
    if (asset)
    {
        Logger::Debug<AssetManager>("[AssetManager] asset reload enqueued: %s", filepath.GetAbsolutePathname().c_str());
        EnqueuePrepareDataForReloadAssetJob(asset);
    }
}

void AssetManager::EnqueuePrepareDataForReloadAssetJob(Asset<AssetBase> asset)
{
    GetEngineContext()->jobManager->CreateWorkerJob(Bind(MakeFunction(this, &AssetManager::PrepareDataForReloadJob), asset));
}

void AssetManager::EnqueueLoadAssetJob(Asset<AssetBase> asset)
{
    GetEngineContext()->jobManager->CreateWorkerJob(Bind(MakeFunction(this, &AssetManager::LoadAssetJob), asset));
}

void AssetManager::ReloadAssetJob(Asset<AssetBase> asset)
{
    asset->Reload();
}

void AssetManager::PrepareDataForReloadJob(Asset<AssetBase> asset)
{
    /*
    // AssetBase* basePointer = asset.get();
    //Reflection ref = Reflection::Create(basePointer);
    //const Type* type = ref.GetValueType();
    
    // Started code to recreate asset using it's reflected type. But not finished.
     
    const ReflectedType* rType = ReflectedTypeDB::GetByPointer(basePointer);
    if (rType)
    {
        Logger::Debug("[AssetManager] reload job type: %s", rType->GetPermanentName().c_str());
    }
    */
    asset->PrepareDataForReload(asset->GetFilepath());
    GetEngineContext()->jobManager->CreateMainJob(Bind(MakeFunction(this, &AssetManager::ReloadAssetJob), asset));
}

void AssetManager::LoadAssetJob(Asset<AssetBase> asset)
{
    asset->SetState(AssetBase::LOADING);
    asset->Load(asset->GetFilepath());
    asset->SetState(AssetBase::LOADED);
    AssetLoadedFunction loadedCallback = asset->GetAssetDescriptor()->assetLoadedCallback;
    if (loadedCallback)
    {
        GetEngineContext()->jobManager->CreateMainJob(Bind(loadedCallback, asset));
    }
}
}
