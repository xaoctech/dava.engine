#pragma once

#include "Classes/Constants.h"

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class DebugDrawData;

class DebugDrawModule : public DAVA::TArc::ClientModule
{
protected:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void PostInit() override;

private:
    DAVA::TArc::QtConnections connections;

    void OnHangingObjects();
    void OnHangingObjectsHeight(double value);
    void OnSwitchWithDifferentLODs();

    bool IsDisabled() const;
    ResourceEditor::eSceneObjectType DebugDrawObject() const;
    void SetDebugDrawObject(ResourceEditor::eSceneObjectType type);

    void ChangeObject(ResourceEditor::eSceneObjectType object);

    DAVA_VIRTUAL_REFLECTION(DebugDrawModule, DAVA::TArc::ClientModule);
};
