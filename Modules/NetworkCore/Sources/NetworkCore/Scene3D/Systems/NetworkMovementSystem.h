#pragma once
#include <Entity/SceneSystem.h>
#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Scene3D/EntityGroup.h>
#include <Scene3D/Systems/BaseSimulationSystem.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class NetworkTimeSingleComponent;
class NetworkMovementComponent;
class NetworkMovementSettings;
class NetworkMovementSystem : public BaseSimulationSystem
{
    DAVA_VIRTUAL_REFLECTION(NetworkMovementSystem, BaseSimulationSystem);

public:
    NetworkMovementSystem(Scene* scene);
    ~NetworkMovementSystem();

    void Process(float32 timeElapsed) override;
    void ProcessFixed(float32 timeElapsed) override;

    void ReSimulationStart() override;
    void ReSimulationEnd() override;

private:
    bool isResimulation = false;
    const NetworkMovementSettings* settings = nullptr;

    EntityGroup* transformAndMovementGroup = nullptr;
    const NetworkTimeSingleComponent* timeSingleComponent = nullptr;

    void OnMovementEntityAdded(Entity*);

    void ApplyInterpolation(Entity* entity, float32 overlap, float32 timeElapsed);
    void ApplySmoothCorrection(Entity* entity, float32 timeElapsed);

    void AppendInterpolatedPosHistory(const Vector3& prevPos, const Vector3& pos, float32 timeElapsed);
    void DrawInterpolatedPosHistory();

    float32 GetInteplocationPos(NetworkMovementComponent* nmc);
    Vector3 GetSmoothCorrectedPosition(const Vector3& orig, float32 interpolationPosk, NetworkMovementComponent* nmc);
    Quaternion GetSmoothCorrectedRotation(const Quaternion& orig, float32 interpolationPos, NetworkMovementComponent* nmc);
};

} // namespace DAVA
