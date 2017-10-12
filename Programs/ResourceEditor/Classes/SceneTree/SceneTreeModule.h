#pragma once

#include <TArc/Core/ClientModule.h>

#include <Reflection/Reflection.h>

class SceneTreeModule : public DAVA::TArc::ClientModule
{
private:
    void OnContextCreated(DAVA::TArc::DataContext* context) override;
    void OnContextDeleted(DAVA::TArc::DataContext* context) override;

    void PostInit() override;

    SceneTreeModelV2* GetDataModel() const;
    QItemSelectionModel* GetSelectionModel() const;

private:
    DAVA_VIRTUAL_REFLECTION(SceneTreeModule, DAVA::TArc::ClientModule);
};
