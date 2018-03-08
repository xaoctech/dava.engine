#pragma once

#include <Entity/SingleComponent.h>

namespace DAVA
{
class IServer;

class NetworkServerSingleComponent : public SingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkServerSingleComponent, SingleComponent);

    NetworkServerSingleComponent(IServer* server = nullptr);
    ~NetworkServerSingleComponent();

    void SetServer(IServer* server);
    IServer* GetServer() const;

private:
    IServer* server;
};
}
