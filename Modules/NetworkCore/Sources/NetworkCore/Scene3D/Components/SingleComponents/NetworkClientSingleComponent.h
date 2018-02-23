#pragma once

#include <Entity/SingletonComponent.h>

namespace DAVA
{
class IClient;

class NetworkClientSingleComponent : public SingletonComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkClientSingleComponent, SingletonComponent);

    NetworkClientSingleComponent(IClient* client = nullptr);
    ~NetworkClientSingleComponent();

    void SetClient(IClient* client);
    IClient* GetClient() const;

private:
    IClient* client;
};
}
