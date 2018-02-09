#include "NetworkServerSingleComponent.h"

#include "NetworkCore/UDPTransport/UDPServer.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkServerSingleComponent)
{
    ReflectionRegistrator<NetworkServerSingleComponent>::Begin()
    .ConstructorByPointer()
    .ConstructorByPointer<IServer*>()
    .End();
}

NetworkServerSingleComponent::NetworkServerSingleComponent(IServer* server_)
    : server(server_)
{
}

NetworkServerSingleComponent::~NetworkServerSingleComponent()
{
    server = nullptr;
}

void NetworkServerSingleComponent::SetServer(IServer* server_)
{
    if (server == nullptr)
    {
        server = server_;
    }
    else
    {
        DVASSERT(false, "Server already set. Remove this assert if it's time to add server changing logic.");
    }
}

IServer* NetworkServerSingleComponent::GetServer() const
{
    return server;
}
}
