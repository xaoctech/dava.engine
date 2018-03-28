#pragma once

#include <Math/Color.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>

namespace DAVA
{
class Color;
class Texture;
class PolygonGroup;
class DynamicBodyComponent;
} // namespace DAVA

/** Responsible for filling new entities for `Cubes` game mode. */
class CubesEntityFillSystem final : public DAVA::BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(CubesEntityFillSystem, DAVA::BaseSimulationSystem);

public:
    CubesEntityFillSystem(DAVA::Scene* scene);
    ~CubesEntityFillSystem();

    void ProcessFixed(DAVA::float32 timeElapsed) override;

private:
    void FillRenderObject(DAVA::Entity* cube, DAVA::PolygonGroup* polygonGroup, const DAVA::Color& color) const;
    void FillBigCube(DAVA::Entity* bigCube) const;
    void FillSmallCube(DAVA::Entity* smallCube) const;
    DAVA::PolygonGroup* CreateCubePolygonGroup(DAVA::float32 size) const;
    void UpdateBodiesColors(DAVA::float32 timeElapsed);

    DAVA::EntityGroupOnAdd* pendingBigCubes = nullptr;
    DAVA::EntityGroupOnAdd* pendingSmallCubes = nullptr;

    DAVA::ComponentGroup<DAVA::DynamicBodyComponent>* dynamicBodies = nullptr;

    const DAVA::float32 fadeTime = 3.5f;

    DAVA::UnorderedMap<DAVA::DynamicBodyComponent*, DAVA::float32> timeInfo;

    DAVA::Texture* cubeTexture = nullptr;
    DAVA::PolygonGroup* smallCubePolygonGroup = nullptr;
    DAVA::PolygonGroup* bigCubePolygonGroup = nullptr;

    const DAVA::Color smallCubeColor = std::stoul("0xbcbcbcff", nullptr, 16);
    const DAVA::Color bigCubeColor = std::stoul("0x2864d3ff", nullptr, 16);
};
