#ifdef __DAVAENGINE_BEAST__

#include "BeastProxyImpl.h"
#include "BeastManager.h"
#include "BeastTexture.h"

BeastManager* BeastProxyImpl::CreateManager()
{
    return new BeastManager();
}

void BeastProxyImpl::SafeDeleteManager(BeastManager** manager)
{
    delete (*manager);
    *manager = nullptr;
}

void BeastProxyImpl::Run(BeastManager* manager, DAVA::Scene* scene)
{
    manager->Run(scene);
}

void BeastProxyImpl::SetLightmapsDirectory(BeastManager* manager, const DAVA::FilePath& path)
{
    manager->SetLightmapsDirectory(path.GetAbsolutePathname());
}

void BeastProxyImpl::SetMode(BeastManager* manager, BeastProxy::eBeastMode mode)
{
    manager->SetMode(mode);
}

void BeastProxyImpl::Update(BeastManager* manager)
{
    if (manager)
    {
        manager->Update();
    }
}

bool BeastProxyImpl::IsJobDone(BeastManager* manager)
{
    if (manager)
    {
        return manager->IsJobDone();
    }

    return false;
}

void BeastProxyImpl::UpdateAtlas(BeastManager* manager, DAVA::Vector<LightmapAtlasingData>* atlasData)
{
    if (manager)
    {
        manager->UpdateAtlas(atlasData);
    }
}

void BeastProxyImpl::Cancel(BeastManager* manager)
{
    if (manager)
    {
        manager->Cancel();
    }
}

bool BeastProxyImpl::WasCancelled(BeastManager* manager) const
{
    if (manager)
    {
        return manager->WasCancelled();
    }

    return false;
}

int BeastProxyImpl::GetCurTaskProcess(BeastManager* manager) const
{
    int ret = 0;

    if (manager)
    {
        ret = manager->GetCurTaskProcess();
    }

    return ret;
};

DAVA::String BeastProxyImpl::GetCurTaskName(BeastManager* manager) const
{
    DAVA::String ret;

    if (manager)
    {
        ret = manager->GetCurTaskName();
    }

    return ret;
};


#endif //__DAVAENGINE_BEAST__
