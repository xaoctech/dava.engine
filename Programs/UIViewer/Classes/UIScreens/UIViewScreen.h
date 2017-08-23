#pragma once

#include "UIScreens/BaseScreen.h"

#include <FileSystem/FilePath.h>

namespace DAVA
{
class ProgramOptions;
}

class UIViewScreen : public BaseScreen
{
public:
    UIViewScreen(DAVA::Window* window, DAVA::ProgramOptions* options);

    void LoadResources() override;
    void UnloadResources() override;

private:
    void SetupUI();

    void PrintError(const DAVA::String& errorMessage);

    DAVA::Window* window = nullptr;
    DAVA::ProgramOptions* options = nullptr;

    DAVA::FilePath projectPath;
};
