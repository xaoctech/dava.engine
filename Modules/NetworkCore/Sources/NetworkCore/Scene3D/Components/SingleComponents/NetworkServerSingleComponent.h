#pragma once

#include <Entity/SingletonComponent.h>

namespace DAVA
{
class IServer;

class NetworkServerSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkServerSingleComponent, SingletonComponent);

    NetworkServerSingleComponent(IServer* server = nullptr);
    ~NetworkServerSingleComponent();

    void SetServer(IServer* server);
    IServer* GetServer() const;

private:
    IServer* server;

    void Clear() override;
};
}
