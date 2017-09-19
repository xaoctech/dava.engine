#pragma once

#include "Classes/Modules/QEClientModule.h"

namespace DAVA
{
class Any;
}
class ControlNode;

class DistanceLinesModule : public QEClientModule
{
public:
    DistanceLinesModule();

private:
    void PostInit() override;
    void CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;
    void DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager) override;

    ControlNode* GetHighlightNode() const;

    DAVA_VIRTUAL_REFLECTION(DistanceLinesModule, DAVA::TArc::ClientModule);
};
