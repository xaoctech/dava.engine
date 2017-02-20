#pragma once

#include <UI/Components/UIComponent.h>
#include <FileSystem/FilePath.h>

namespace DAVA
{

class UISpineComponent : public UIBaseComponent<UISpineComponent>
{
public:
    UISpineComponent() = default;
    UISpineComponent(const UISpineComponent& copy) = default;
    UISpineComponent& operator=(const UISpineComponent&) = delete;
    UISpineComponent* Clone() const override;

    const FilePath& GetSkeketonPath() const;
    void SetSkeketonPath(const FilePath& path);

protected:
    ~UISpineComponent() override = default;

private:
    FilePath skeletonPath;
    Map<String, String> controlsToBones;
};

}