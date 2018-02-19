#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>

public:
DAVA_VIRTUAL_REFLECTION(TEMPLATESystem, SceneSystem);

TEMPLATESystem(Scene* scene);

void ProcessFixed(float32 timeElapsed) override;
void PrepareForRemove() override{};
}
;

} //namespace DAVA
