#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIDataViewModelComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIDataViewModelComponent, UIComponent);

public:
    DECLARE_UI_COMPONENT(UIDataViewModelComponent);

    UIDataViewModelComponent() = default;
    UIDataViewModelComponent(const UIDataViewModelComponent& c);

    UIDataViewModelComponent& operator=(const UIDataViewModelComponent&) = delete;

    UIDataViewModelComponent* Clone() const override;

    const FilePath& GetViewModelFile() const;
    void SetViewModelFile(const FilePath& path);

    bool IsDirty() const;
    void SetDirty(bool dirty);

protected:
    ~UIDataViewModelComponent() override = default;

private:
    FilePath viewModelFile;
    bool isDirty = false;
};
}
