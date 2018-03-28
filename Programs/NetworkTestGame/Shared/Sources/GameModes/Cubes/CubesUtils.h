#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>

namespace DAVA
{
class Scene;
class Entity;
} // namespace DAVA

namespace CubesDetails
{
using namespace DAVA;

static const uint32 WallCollisionShapeType = 1 << 0;
static const uint32 CubeCollisionShapeType = 1 << 1;
static const float32 RoomSize = 30.0f;

static const FastName ActionForward = FastName("FORWARD");
static const FastName ActionBack = FastName("BACK");
static const FastName ActionLeft = FastName("LEFT");
static const FastName ActionRight = FastName("RIGHT");
static const FastName ActionUp = FastName("UP");
static const FastName ActionFastUp = FastName("FIRST_SHOOT"); // For UI

static const float32 SmallCubeHalfSize = 0.25f;
static const float32 BigCubeHalfSize = 0.75f;
} // namespace CubesDetails

class CubesUtils final
{
public:
    static void SetupCubesScene(DAVA::Scene* scene);

private:
    static void FillSceneWithSmallCubes(DAVA::Scene* scene);
};