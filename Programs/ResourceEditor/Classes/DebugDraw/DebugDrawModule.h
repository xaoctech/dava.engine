#pragma once

#include <REPlatform/Global/Constants.h>

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class DebugDrawData;

class DebugDrawModule : public DAVA::ClientModule
{
protected:
    void OnContextCreated(DAVA::DataContext* context) override;
    void OnContextDeleted(DAVA::DataContext* context) override;

    void PostInit() override;

private:
    DAVA::QtConnections connections;
    std::unique_ptr<DAVA::FieldBinder> fieldBinder;

    void OnHangingObjects();
    void OnHangingObjectsHeight(double value);
    void OnSwitchWithDifferentLODs();

    bool IsDisabled() const;
    DAVA::ResourceEditor::eSceneObjectType DebugDrawObject() const;
    void SetDebugDrawObject(DAVA::ResourceEditor::eSceneObjectType type);

    void ChangeObject(DAVA::ResourceEditor::eSceneObjectType object);

    DAVA_VIRTUAL_REFLECTION(DebugDrawModule, DAVA::ClientModule);
};
