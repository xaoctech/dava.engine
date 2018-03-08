#pragma once

#include <Entity/SingleComponent.h>

namespace DAVA
{
class IClient;

class NetworkClientSingleComponent : public SingleComponent
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkClientSingleComponent, SingleComponent);

    NetworkClientSingleComponent(IClient* client = nullptr);
    ~NetworkClientSingleComponent();

    void SetClient(IClient* client);
    IClient* GetClient() const;

private:
    IClient* client;
};
}
