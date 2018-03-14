#pragma once

#include "REPlatform/DataNodes/SelectableGroup.h"
#include "REPlatform/Scene/Systems/EditorSceneSystem.h"
#include "REPlatform/Scene/Systems/SystemDelegates.h"

#include <Entity/SceneSystem.h>
#include <Render/RenderHelper.h>
#include <Scene3D/Components/SplineComponent.h>
#include <UI/UIEvent.h>

namespace DAVA
{
class SplineEditorDrawComponent;

class SplineEditorSystem : public SceneSystem,
                           public EditorSceneSystem,
                           public SelectionSystemDelegate
{
public:
    SplineEditorSystem(Scene* scene);

    // SceneSystem
    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void Process(float32 timeElapsed) override;
    bool Input(UIEvent* event) override;

    // EditorSceneSystem
    void PrepareForRemove() override;

    // SelectionSystemDelegate
    bool AllowAddToSelection(const Selectable& itemToAdd) override;

    const UnorderedSet<SplineEditorDrawComponent*>& GetSplineDrawComponents() const;
    SplineEditorDrawComponent* GetSplineDrawByPoint(SplineComponent::SplinePoint* point) const;
    SplineComponent* GetSplineByPoint(SplineComponent::SplinePoint* point) const;

    void AddPointByShiftAndClick();
    void ChangePointValueByWheel(UIEvent* event, bool shiftPressed);

private:
    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

private:
    UnorderedSet<SplineEditorDrawComponent*> splineDrawComponents;
    UnorderedMap<SplineComponent::SplinePoint*, SplineComponent*> pointsToSplines;
    SelectableGroup shouldBeSelectedOnNextFrame;
};
} // namespace DAVA
