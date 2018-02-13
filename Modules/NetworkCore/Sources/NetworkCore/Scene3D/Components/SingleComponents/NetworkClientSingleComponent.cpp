#include "NetworkClientSingleComponent.h"

#include "NetworkCore/UDPTransport/UDPClient.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkClientSingleComponent)
{
    ReflectionRegistrator<NetworkClientSingleComponent>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<IClient*>()
    .End();
}

NetworkClientSingleComponent::NetworkClientSingleComponent(IClient* client_)
    : client(client_)
{
}

NetworkClientSingleComponent::~NetworkClientSingleComponent()
{
    client = nullptr;
}

void NetworkClientSingleComponent::SetClient(IClient* client_)
{
    if (client == nullptr)
    {
        client = client_;
    }
    else
    {
        DVASSERT(false, "Client already set.");
    }
}

IClient* NetworkClientSingleComponent::GetClient() const
{
    return client;
}

void NetworkClientSingleComponent::Clear()
{
}

} // namespace DAVA
