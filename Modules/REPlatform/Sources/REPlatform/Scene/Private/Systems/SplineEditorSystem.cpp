#include "REPlatform/Scene/Systems/SplineEditorSystem.h"

#include "REPlatform/Commands/RECommandNotificationObject.h"
#include "REPlatform/Commands/SplineEditorCommands.h"
#include "REPlatform/Commands/SetFieldValueCommand.h"
#include "REPlatform/Commands/TransformCommand.h"
#include "REPlatform/Scene/Components/SplineEditorDrawComponent.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"

#include <Debug/DVAssert.h>
#include <DeviceManager/DeviceManager.h>
#include <Input/Keyboard.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/SingleComponents/VTSingleComponent.h>
#include <Scene3D/Components/SplineComponent.h>
#include <Scene3D/Components/VTDecalComponent.h>
#include <Scene3D/Systems/VTSplineDecalSystem.h>
#include <Utils/Utils.h>

namespace DAVA
{
SplineEditorSystem::SplineEditorSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<SplineComponent>())
{
}

void SplineEditorSystem::RegisterEntity(Entity* entity)
{
    using namespace DAVA;

    SplineComponent* splineComponent = GetSplineComponent(entity);
    if (splineComponent != nullptr)
    {
        SplineEditorDrawComponent* splineDrawComponent = new SplineEditorDrawComponent;
        splineDrawComponents.insert(splineDrawComponent);
        entity->AddComponent(splineDrawComponent);
        for (SplineComponent::SplinePoint* point : splineComponent->controlPoints)
        {
            pointsToSplines.emplace(point, splineComponent);
        }
    }
}

void SplineEditorSystem::UnregisterEntity(Entity* entity)
{
    SplineComponent* splineComponent = GetSplineComponent(entity);
    if (splineComponent != nullptr)
    {
        SplineEditorDrawComponent* drawComponent = entity->GetComponent<SplineEditorDrawComponent>();
        splineDrawComponents.erase(drawComponent);

        for (SplineComponent::SplinePoint* point : splineComponent->controlPoints)
        {
            pointsToSplines.erase(point);
        }
    }
}

void SplineEditorSystem::RegisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Type::Instance<SplineComponent>())
    {
        RegisterEntity(entity);
    }
}

void SplineEditorSystem::UnregisterComponent(Entity* entity, Component* component)
{
    if (component->GetType() == Type::Instance<SplineComponent>())
    {
        UnregisterEntity(entity);
    }
}

void SplineEditorSystem::PrepareForRemove()
{
}

bool SplineEditorSystem::AllowAddToSelection(const Selectable& itemToAdd)
{
    bool noSnappedSplines = std::none_of(splineDrawComponents.begin(), splineDrawComponents.end(), [](const SplineEditorDrawComponent* c) { return c->IsSnapped(); });
    return noSnappedSplines || (itemToAdd.CanBeCastedTo<SplineComponent::SplinePoint>());
}

const DAVA::UnorderedSet<SplineEditorDrawComponent*>& SplineEditorSystem::GetSplineDrawComponents() const
{
    return splineDrawComponents;
}

SplineEditorDrawComponent* SplineEditorSystem::GetSplineDrawByPoint(SplineComponent::SplinePoint* point) const
{
    SplineComponent* spline = GetSplineByPoint(point);
    return (spline != nullptr ? spline->GetEntity()->GetComponent<SplineEditorDrawComponent>() : nullptr);
}

DAVA::SplineComponent* SplineEditorSystem::GetSplineByPoint(SplineComponent::SplinePoint* point) const
{
    auto found = pointsToSplines.find(point);
    return (found != pointsToSplines.end()) ? found->second : nullptr;
}

void SplineEditorSystem::Process(float32 timeElapsed)
{
    if (shouldBeSelectedOnNextFrame.IsEmpty() == false)
    {
        SelectionSystem* selectionSystem = GetScene()->GetSystem<SelectionSystem>();
        selectionSystem->SetSelectionAllowed(true);
        selectionSystem->SetSelection(shouldBeSelectedOnNextFrame);
        shouldBeSelectedOnNextFrame.Clear();
    }
}

bool SplineEditorSystem::Input(UIEvent* event)
{
    if (eMouseButtons::LEFT == event->mouseButton && UIEvent::Phase::BEGAN == event->phase)
    {
        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        bool shiftPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || kb->GetKeyState(eInputElements::KB_RSHIFT).IsPressed());
        if (shiftPressed)
        {
            AddPointByShiftAndClick();
        }
    }
    else if (event->phase == UIEvent::Phase::WHEEL)
    {
        Keyboard* kb = GetEngineContext()->deviceManager->GetKeyboard();
        bool shiftPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LSHIFT).IsPressed() || kb->GetKeyState(eInputElements::KB_RSHIFT).IsPressed());
        bool ctrlPressed = (kb != nullptr) && (kb->GetKeyState(eInputElements::KB_LCTRL).IsPressed() || kb->GetKeyState(eInputElements::KB_RCTRL).IsPressed());
        if (shiftPressed || ctrlPressed)
        {
            ChangePointValueByWheel(event, shiftPressed);
        }
    }
    return false;
}

void SplineEditorSystem::AddPointByShiftAndClick()
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());
    SelectionSystem* selectionSystem = scene->GetSystem<SelectionSystem>();
    const SelectableGroup& selectableGroup = selectionSystem->GetSelection();

    SplineComponent* spline = nullptr;
    SplineComponent::SplinePoint* selectedPoint = nullptr;

    if (selectableGroup.GetSize() == 1)
    {
        const Selectable& selectable = selectableGroup.GetFirst();
        Entity* e = selectable.AsEntity();
        if (e != nullptr)
        {
            spline = GetSplineComponent(e);
            if (spline == nullptr || spline->GetControlPoints().empty() == false)
            {
                return;
            }
        }
        else if (selectable.GetObjectType() == ReflectedTypeDB::Get<SplineComponent::SplinePoint>())
        {
            selectedPoint = selectable.Cast<SplineComponent::SplinePoint>();

            auto found = pointsToSplines.find(selectedPoint);
            DVASSERT(found != pointsToSplines.end());
            spline = found->second;
        }
    }

    if (spline == nullptr && selectedPoint == nullptr)
    {
        return;
    }

    Vector3 landscapeIntersectionPos;
    SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();
    bool landscapeIntersected = collisionSystem->LandRayTestFromCamera(landscapeIntersectionPos);
    if (landscapeIntersected)
    {
        selectionSystem->SetSelectionAllowed(false);

        Matrix4 parentTransform = spline->GetEntity()->GetWorldTransform();
        parentTransform.Inverse();

        Matrix4 newPointTransform;
        newPointTransform.SetTranslationVector(landscapeIntersectionPos);
        newPointTransform = newPointTransform * parentTransform;

        size_t newPointIndex = 0;
        SplineComponent::SplinePoint* newPoint = new SplineComponent::SplinePoint;
        newPoint->position = newPointTransform.GetTranslationVector();
        if (selectedPoint != nullptr)
        {
            newPoint->value = selectedPoint->value;
            newPoint->width = selectedPoint->width;
            newPointIndex = GetIndexOfElement(spline->controlPoints, selectedPoint) + 1;
        }

        scene->Exec(std::make_unique<AddSplinePointCommand>(scene, spline, newPoint, newPointIndex));
    }
}

void SplineEditorSystem::ChangePointValueByWheel(UIEvent* event, bool shiftPressed)
{
    SceneEditor2* scene = static_cast<SceneEditor2*>(GetScene());

    SelectionSystem* selectionSystem = scene->GetSystem<SelectionSystem>();
    const SelectableGroup& selectableGroup = selectionSystem->GetSelection();

    if (selectableGroup.ContainsOnlyObjectsOfType<SplineComponent::SplinePoint>())
    {
        scene->BeginBatch("Change Spline Points values", selectableGroup.GetSize());

        for (const Selectable& selectable : selectableGroup.GetContent())
        {
            SplineComponent::SplinePoint* point = selectable.Cast<SplineComponent::SplinePoint>();
            SplineComponent* spline = GetSplineByPoint(point);
            float32 origEntry = shiftPressed ? point->width : point->value;
            float32 wheelStep = origEntry > 1.f ? 1.f : 0.1f;

            float32 newEntry = origEntry + (event->wheelDelta.y * wheelStep);
            if (newEntry < 0.f)
            {
                newEntry = 0.f;
            }
            else if (newEntry < 1.f && origEntry > 1.f)
            {
                newEntry = 1.f;
            }

            float32 newWidth = shiftPressed ? newEntry : point->width;
            float32 newValue = shiftPressed ? point->value : newEntry;
            scene->Exec(std::make_unique<ChangeSplinePointCommand>(point, newWidth, newValue));
        }

        scene->EndBatch();
        event->wheelDelta.y = 0.f;
    }
}

void SplineEditorSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    auto fn = [this](const SplinePointAddRemoveCommand* cmd)
    {
        if (cmd->IsPointInserted())
        {
            pointsToSplines.emplace(cmd->GetSplinePoint(), cmd->GetSpline());

            shouldBeSelectedOnNextFrame.Clear();
            shouldBeSelectedOnNextFrame.Add(Any(cmd->GetSplinePoint()));
        }
        else
        {
            pointsToSplines.erase(cmd->GetSplinePoint());

            size_t pointIndex = cmd->GetSplinePointIndex();
            if (pointIndex < cmd->GetSpline()->controlPoints.size())
            { // select next point
                shouldBeSelectedOnNextFrame.Clear();
                shouldBeSelectedOnNextFrame.Add(cmd->GetSpline()->controlPoints[pointIndex]);
            }
            else if (pointIndex > 0)
            { // select previous point
                shouldBeSelectedOnNextFrame.Clear();
                shouldBeSelectedOnNextFrame.Add(cmd->GetSpline()->controlPoints[pointIndex - 1]);
            }
        }
        GetScene()->GetSingletonComponent<VTSingleComponent>()->vtSplineChanged.push_back(cmd->GetSpline()->GetEntity()); // todo: use spline single component
    };

    commandNotification.ForEach<AddSplinePointCommand>(fn);
    commandNotification.ForEach<RemoveSplinePointCommand>(fn);

    commandNotification.ForEach<TransformCommand>([this](const TransformCommand* cmd) {
        if (cmd->GetTransformedObject().CanBeCastedTo<SplineComponent::SplinePoint>())
        {
            SplineComponent::SplinePoint* point = cmd->GetTransformedObject().Cast<SplineComponent::SplinePoint>();
            SplineComponent* spline = GetSplineByPoint(point);
            GetScene()->GetSingletonComponent<VTSingleComponent>()->vtSplineChanged.push_back(spline->GetEntity()); // todo: use spline single component
        }
    });

    commandNotification.ForEach<SetFieldValueCommand>([this](const SetFieldValueCommand* cmd) {
        const Vector<Selectable>& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection().GetContent();
        for (const Selectable& selectable : selection)
        {
            if (selectable.CanBeCastedTo<SplineComponent::SplinePoint>())
            {
                SplineComponent::SplinePoint* point = selectable.Cast<SplineComponent::SplinePoint>();
                SplineComponent* spline = GetSplineByPoint(point);
                GetScene()->GetSingletonComponent<VTSingleComponent>()->vtSplineChanged.push_back(spline->GetEntity()); // todo: use spline single component
            }
        }
    });

    commandNotification.ForEach<ChangeSplinePointCommand>([this](const ChangeSplinePointCommand* cmd) {
        SplineComponent* spline = GetSplineByPoint(cmd->GetPoint());
        GetScene()->GetSingletonComponent<VTSingleComponent>()->vtSplineChanged.push_back(spline->GetEntity()); // todo: use spline single component
    });
}

void SplineEditorSystem::Draw()
{
    RenderHelper* debugDrawer = GetScene()->GetRenderSystem()->GetDebugDrawer();

    for (SplineEditorDrawComponent* splineDraw : splineDrawComponents)
    {
        if (splineDraw->IsSplineVisible())
        {
            SplineComponent* spline = GetSplineComponent(splineDraw->GetEntity());

            static const Color solidDepthColorNormal = Color(1.0f, 0.5f, 0.25f, 0.3f);
            static const Color wireDepthColorNormal = Color(1.0f, 0.5f, 0.25f, 1.0f);
            static const Color solidDepthColorSnapped = Color(1.0f, 0.4f, 0.15f, 0.3f);
            static const Color wireDepthColorSnapped = Color(1.0f, 0.4f, 0.15f, 1.0f);

            const Color& solidDepthColor = splineDraw->IsSnapped() ? solidDepthColorSnapped : solidDepthColorNormal;
            const Color& wireDepthColor = splineDraw->IsSnapped() ? wireDepthColorSnapped : wireDepthColorNormal;

            size_t pointsCount = spline->controlPoints.size();
            for (size_t i = 0; i < pointsCount; ++i)
            {
                SplineComponent::SplinePoint* point = spline->controlPoints[i];

                Vector3 startPosition = point->position;
                const AABBox3 splinePointBoundingBox(startPosition, 2.f);
                const auto& transform = spline->GetEntity()->GetWorldTransform();

                debugDrawer->DrawAABoxTransformed(splinePointBoundingBox, transform, solidDepthColor, RenderHelper::DRAW_SOLID_DEPTH);
                debugDrawer->DrawAABoxTransformed(splinePointBoundingBox, transform, wireDepthColor, RenderHelper::DRAW_WIRE_DEPTH);

                if (i < pointsCount - 1)
                {
                    static const float32 POINT_DRAW_LIFTING = 1.f;
                    startPosition.z += POINT_DRAW_LIFTING;
                    Vector3 finishPosition = spline->controlPoints[i + 1]->position;
                    finishPosition.z += POINT_DRAW_LIFTING;

                    startPosition = startPosition * transform;
                    finishPosition = finishPosition * transform;

                    debugDrawer->DrawArrow(startPosition, finishPosition, Min((finishPosition - startPosition).Length() / 6.f, 4.f), ClampToUnityRange(Color(1.0f, 0.5f, 0.25f, 1.f)), RenderHelper::DRAW_SOLID_DEPTH);
                }
            }
        }
    }
}

} // namespace DAVA
