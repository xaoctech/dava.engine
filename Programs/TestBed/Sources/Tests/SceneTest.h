#pragma once

#include "Infrastructure/BaseScreen.h"

class TestBed;
class SceneTest : public BaseScreen
{
public:
    SceneTest(TestBed& app);

protected:
    void LoadResources() override;
    void UnloadResources() override;
};
