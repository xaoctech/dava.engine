#pragma once

namespace DAVA
{
class NetworkMotionComponent;

struct NetworkMotionUtils final
{
    static void CopyToMotion(const NetworkMotionComponent* networkMotionComponent);
};
}