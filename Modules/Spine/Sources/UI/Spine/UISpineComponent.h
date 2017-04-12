#pragma once

#include <FileSystem/FilePath.h>
#include <Reflection/Reflection.h>
#include <UI/Components/UIComponent.h>

namespace DAVA
{

class UISpineComponent : public UIBaseComponent<UISpineComponent>
{
    DAVA_VIRTUAL_REFLECTION(UISpineComponent, UIBaseComponent<UISpineComponent>);

public:
    UISpineComponent() = default;
    UISpineComponent(const UISpineComponent& copy);
    UISpineComponent& operator=(const UISpineComponent&) = delete;
    UISpineComponent* Clone() const override;

    /** Spine skeleton data file */
    const FilePath& GetSkeketonPath() const;
    void SetSkeketonPath(const FilePath& path);

    /** Spine atlas texture file */
    const FilePath& GetAtlasPath() const;
    void SetAtlasPath(const FilePath& path);



protected:
    ~UISpineComponent() override = default;

private:
    FilePath skeletonPath;
    FilePath atlasPath;
};

}