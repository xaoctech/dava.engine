#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Core/FieldBinder.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class SceneTreeModule : public DAVA::TArc::ClientModule
{
private:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void PostInit() override;

    SceneTreeModelV2* GetDataModel() const;
    QItemSelectionModel* GetSelectionModel() const;

    void OnSceneSelectionChanged(const DAVA::Any& value);
    void OnSceneTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected, QItemSelectionModel* selectionModel);

private:
    std::unique_ptr<DAVA::TArc::FieldBinder> fieldBinder;
    DAVA::TArc::QtConnections connections;
    bool inSelectionSync = false;

    DAVA_VIRTUAL_REFLECTION(SceneTreeModule, DAVA::TArc::ClientModule);
};
