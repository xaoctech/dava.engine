#pragma once

#include <memory>

#include "DAVAEngine.h"

class AutoInput;

namespace DAVA
{
class ActionsSingleComponent;
}

class MainScreen : public DAVA::UIScreen
{
public:
    MainScreen(DAVA::Scene* scene_);

    void LoadResources() override;
    void UnloadResources() override;

    void Update(DAVA::float32 timeElapsed) override;

    DAVA::int32 GetScreenID() const
    {
        return 1;
    }

    DAVA::ScopedPtr<DAVA::UI3DView> ui3DView;

private:
    DAVA::Scene* scene;
    DAVA::ActionsSingleComponent* actionsSingleComponent = nullptr;

    DAVA::UIJoypad* moveJoyPAD = nullptr;
    void AddJoypadControl();
};
