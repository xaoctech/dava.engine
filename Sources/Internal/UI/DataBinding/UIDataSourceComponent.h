#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIDataSourceComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIDataSourceComponent, UIComponent);
    DECLARE_UI_COMPONENT(UIDataSourceComponent);

public:
    UIDataSourceComponent() = default;
    UIDataSourceComponent(const String& modelName);
    UIDataSourceComponent(const UIDataSourceComponent& c);

    UIDataSourceComponent& operator=(const UIDataSourceComponent& c) = delete;

    UIDataSourceComponent* Clone() const override;

    const Reflection& GetData() const;
    void SetData(const Reflection& data);

    const FilePath& GetDataFile() const;
    void SetDataFile(const FilePath& path);

    bool IsDirty() const;
    void SetDirty(bool dirty_);

protected:
    ~UIDataSourceComponent() override = default;

private:
    Reflection data;
    FilePath dataFile;
    bool isDirty = false;
};
}
