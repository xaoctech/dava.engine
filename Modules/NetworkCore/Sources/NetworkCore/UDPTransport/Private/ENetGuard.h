#pragma once

namespace DAVA
{
class ENetGuard
{
public:
    ENetGuard();
    ~ENetGuard();

private:
    static unsigned int instanceCount;
};
} // namespace DAVA