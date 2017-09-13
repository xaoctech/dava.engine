#pragma once

#include <TArc/Core/ClientModule.h>

namespace DAVA
{
class Any;
}
class ControlNode;

class DistanceLinesModule : public DAVA::TArc::ClientModule
{
public:
    DistanceLinesModule();

private:
    void PostInit() override;
    void OnInterfaceRegistered(const DAVA::Type* interfaceType) override;
    void OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType) override;

    ControlNode* GetHighlight() const;

    DAVA_VIRTUAL_REFLECTION(DistanceLinesModule, DAVA::TArc::ClientModule);
};
