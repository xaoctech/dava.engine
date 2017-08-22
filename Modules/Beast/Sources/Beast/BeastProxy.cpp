#include "Beast/BeastProxy.h"
#include "Beast/BeastManager.h"
#include "Beast/BeastTexture.h"
#include "Beast/LightmapAtlasingData.h"

BeastManager* BeastProxy::CreateManager()
{
    return new BeastManager();
}

void BeastProxy::SafeDeleteManager(BeastManager** manager)
{
    delete (*manager);
    *manager = nullptr;
}

void BeastProxy::Run(BeastManager* manager, DAVA::Scene* scene)
{
    manager->Run(scene);
}

void BeastProxy::SetLightmapsDirectory(BeastManager* manager, const DAVA::FilePath& path)
{
    manager->SetLightmapsDirectory(path.GetAbsolutePathname());
}

void BeastProxy::SetMode(BeastManager* manager, eBeastMode mode)
{
    manager->SetMode(mode);
}

void BeastProxy::Update(BeastManager* manager)
{
    if (manager)
    {
        manager->Update();
    }
}

bool BeastProxy::IsJobDone(BeastManager* manager)
{
    if (manager)
    {
        return manager->IsJobDone();
    }

    return false;
}

void BeastProxy::UpdateAtlas(BeastManager* manager, DAVA::Vector<LightmapAtlasingData>* atlasData)
{
    if (manager)
    {
        manager->UpdateAtlas(atlasData);
    }
}

void BeastProxy::Cancel(BeastManager* manager)
{
    if (manager)
    {
        manager->Cancel();
    }
}

bool BeastProxy::WasCancelled(BeastManager* manager) const
{
    if (manager)
    {
        return manager->WasCancelled();
    }

    return false;
}

int BeastProxy::GetCurTaskProcess(BeastManager* manager) const
{
    int ret = 0;

    if (manager)
    {
        ret = manager->GetCurTaskProcess();
    }

    return ret;
};

DAVA::String BeastProxy::GetCurTaskName(BeastManager* manager) const
{
    DAVA::String ret;

    if (manager)
    {
        ret = manager->GetCurTaskName();
    }

    return ret;
};
