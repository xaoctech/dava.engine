#pragma once

#include "Classes/ObjectPlacement/Private/ObjectPlacementSystem.h"
#include "Classes/Qt/Scene/System/ModifSystem.h"

#include <TArc/DataProcessing/DataNode.h>
#include <Reflection/ReflectionRegistrator.h>

#include <memory>

class ObjectPlacementData : public DAVA::TArc::DataNode
{
public:
    friend class ObjectPlacementModule;

    static const char* snapToLandscapePropertyName;
    bool GetSnapToLandscape() const;
    void SetSnapToLandscape(bool newSnapToLandscape);
private:
    std::unique_ptr<ObjectPlacementSystem> objectPlacementSystem;

    DAVA_VIRTUAL_REFLECTION(ObjectPlacementData, DAVA::TArc::DataNode);
};
