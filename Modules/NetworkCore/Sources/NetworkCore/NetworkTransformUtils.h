#pragma once

namespace DAVA
{
class NetworkTransformComponent;

struct NetworkTransformUtils final
{
    static void CopyToTransform(const NetworkTransformComponent* networkTransformComponent);
};
}