#pragma once
#include "Asset/Asset.h"
#include "Base/BaseTypes.h"
#include "Functional/Function.h"
#include "Reflection/Reflection.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
class AssetManager;
class AssetBase;

using AssetLoadedFunction = Function<void(Asset<AssetBase> asset)>;

struct AssetDescriptor
{
    AssetDescriptor(const FilePath& assetName_,
                    AssetLoadedFunction assetLoadedCallback_);
    FilePath assetName;
    AssetLoadedFunction assetLoadedCallback;
};

class AssetDependencyWatcher //
{
};

class AssetBase : public ReflectionBase
{
public:
    DAVA_VIRTUAL_REFLECTION(AssetBase);

    struct Dependency
    {
        enum eType
        {
            INCLUDE, // Required for correct functioning
            LINK,
        };
        FilePath filepath;
    };

    enum eState
    {
        EMPTY,
        LOADING,
        LOADED,
        ERROR,
        /*
            Probably required some state flags for time when asset is being reloaded.
         */
    };

    AssetBase();
    virtual ~AssetBase() = default;

    virtual void Load(const FilePath& filepath_){};
    virtual void Save(const FilePath& filepath_){};

    virtual void PrepareDataForReload(const FilePath& filepath_)
    {
    }
    virtual void Reload(){};

    eState GetState() const;
    const Vector<Dependency>& GetDependencies() const;
    const FilePath& GetFilepath() const;

    void SetAssetDescriptor(AssetDescriptor* assetDescriptor_);
    AssetDescriptor* GetAssetDescriptor() const;

private:
    void SetState(eState state_);

    eState state = EMPTY;
    std::unique_ptr<AssetDescriptor> descriptor;
    Vector<Dependency> dependencies;
    friend class AssetManager;
};
};
