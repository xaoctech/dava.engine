#include "Classes/SplineEditor/Private/SplineEditorWidget.h"

#include "Classes/PropertyPanel/PropertyPanelCommon.h"
#include "Classes/SplineEditor/Private/SplineEditorPPExtensions.h"

#include <REPlatform/Commands/SplineEditorCommands.h>
#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Scene/Components/SplineEditorDrawComponent.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/SplineEditorSystem.h>

#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/QtBoxLayouts.h>

#include <Base/Any.h>
#include <Functional/Function.h>
#include <Scene3D/Components/SplineComponent.h>
#include <Utils/Utils.h>

#include <QPushButton>

namespace SplineEditorWidgetDetails
{
using namespace DAVA;

class SplineEditorComponentValue : public BaseComponentValue
{
public:
    SplineEditorComponentValue(Entity* e, SplineEditorDrawComponent* c)
        : drawComponent(c)
        , entity(e)
    {
    }

    bool IsSpannedControl() const override
    {
        return true;
    }

    bool RepaintOnUpdateRequire() const override
    {
        return true;
    }

private:
    Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override
    {
        return false;
    }

    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
        Widget* container = new Widget(parent);
        QWidget* containerWidget = container->ToWidgetCast();
        QtVBoxLayout* mainLayout = new QtVBoxLayout(containerWidget);
        mainLayout->setSpacing(2);
        mainLayout->setMargin(2);
        container->SetLayout(mainLayout);

        QPushButton* button = new QPushButton("Snap Spline", containerWidget);
        button->setToolTip("Snapped splines are always visible");
        button->setFixedWidth(100);
        button->setCheckable(true);
        button->setChecked(drawComponent->IsSnapped());
        connections.AddConnection(button, &QAbstractButton::toggled, DAVA::Bind(&SplineEditorComponentValue::OnButtonToggled, this, _1));
        mainLayout->addWidget(button);
        mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

        return container;
    }

    void OnButtonToggled(bool checked)
    {
        drawComponent->SetSnapped(checked);
        DataContext* context = GetAccessor()->GetActiveContext();
        SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
        SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();
        collisionSystem->UpdateCollisionObject(Selectable(entity), true);
    }

private:
    QtConnections connections;
    SplineEditorDrawComponent* drawComponent = nullptr;
    Entity* entity = nullptr;
};

class SplinePointEditorComponentValue : public BaseComponentValue
{
public:
    bool IsSpannedControl() const override
    {
        return true;
    }

    bool RepaintOnUpdateRequire() const override
    {
        return true;
    }

private:
    Any GetMultipleValue() const override
    {
        return DAVA::Any();
    }

    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override
    {
        return false;
    }

    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override
    {
        bool isAddEnabled = true;

        if (nodes.size() > 1)
        {
            isAddEnabled = false;
        }
        else
        {
            DataContext* context = GetAccessor()->GetActiveContext();
            SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
            SplineEditorSystem* system = scene->GetSystem<SplineEditorSystem>();
            SplineComponent::SplinePoint* point = nodes[0]->cachedValue.Get<SplineComponent::SplinePoint*>();
            SplineComponent* spline = system->GetSplineByPoint(point);
            size_t pointIndex = GetIndexOfElement(spline->controlPoints, point);
            isAddEnabled = (pointIndex + 1) < spline->controlPoints.size();
        }

        Widget* container = new Widget(parent);
        QWidget* containerWidget = container->ToWidgetCast();
        QtHBoxLayout* mainLayout = new QtHBoxLayout(containerWidget);
        mainLayout->setSpacing(2);
        mainLayout->setMargin(2);
        container->SetLayout(mainLayout);

        {
            QPushButton* button = new QPushButton("Add Spline Point", containerWidget);
            button->setToolTip("Add point between selected point and the next point");
            button->setFixedWidth(150);
            button->setEnabled(isAddEnabled);
            connections.AddConnection(button, &QAbstractButton::clicked, DAVA::Bind(&SplinePointEditorComponentValue::OnAddPointClicked, this));
            mainLayout->addWidget(button);
        }

        {
            QString text = (nodes.size() > 1) ? "Remove Spline Points" : "Remove Spline Point";
            QPushButton* button = new QPushButton(text, containerWidget);
            button->setFixedWidth(150);
            connections.AddConnection(button, &QAbstractButton::clicked, DAVA::Bind(&SplinePointEditorComponentValue::OnRemovePointClicked, this));
            mainLayout->addWidget(button);
            mainLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
        }

        return container;
    }

    void OnAddPointClicked()
    {
        DataContext* context = GetAccessor()->GetActiveContext();
        SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
        SplineEditorSystem* system = scene->GetSystem<SplineEditorSystem>();
        SplineComponent::SplinePoint* point = nodes[0]->cachedValue.Get<SplineComponent::SplinePoint*>();
        SplineComponent* spline = system->GetSplineByPoint(point);
        size_t pointIndex = GetIndexOfElement(spline->controlPoints, point);
        SplineComponent::SplinePoint* nextPoint = spline->controlPoints[pointIndex + 1];

        SplineComponent::SplinePoint* newPoint = new SplineComponent::SplinePoint;
        newPoint->position = (point->position + nextPoint->position) / 2;
        newPoint->value = point->value;
        newPoint->width = point->width;

        scene->Exec(std::make_unique<AddSplinePointCommand>(scene, spline, newPoint, pointIndex + 1));
    }

    void OnRemovePointClicked()
    {
        DataContext* context = GetAccessor()->GetActiveContext();
        SceneEditor2* scene = context->GetData<SceneData>()->GetScene().Get();
        SplineEditorSystem* system = scene->GetSystem<SplineEditorSystem>();

        scene->BeginBatch("Remove Spline Points", static_cast<uint32>(nodes.size()));

        for (std::shared_ptr<PropertyNode>& node : nodes)
        {
            SplineComponent::SplinePoint* point = node->cachedValue.Get<SplineComponent::SplinePoint*>();
            SplineComponent* spline = system->GetSplineByPoint(point);
            scene->Exec(std::make_unique<RemoveSplinePointCommand>(scene, spline, point));
        }

        scene->EndBatch();
    }

private:
    QtConnections connections;
};
} // namespace SplineEditorWidgetDetails

std::unique_ptr<DAVA::BaseComponentValue> SplineEditorCreator::GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const
{
    using namespace DAVA;

    if (node->propertyType == PropertyPanel::SplineObject)
    {
        std::shared_ptr<const PropertyNode> splineComponentNode = node->parent.lock();
        DVASSERT(splineComponentNode != nullptr);
        std::shared_ptr<const PropertyNode> entityNode = splineComponentNode->parent.lock();
        DVASSERT(entityNode != nullptr);
        Entity* entity = entityNode->cachedValue.Get<Entity*>();
        SplineEditorDrawComponent* drawComponent = entity->GetComponent<SplineEditorDrawComponent>();

        return std::make_unique<SplineEditorWidgetDetails::SplineEditorComponentValue>(entity, drawComponent);
    }

    return EditorComponentExtension::GetEditor(node);
}

void SplineComponentChildCreator::ExposeChildren(const std::shared_ptr<DAVA::PropertyNode>& node, DAVA::Vector<std::shared_ptr<DAVA::PropertyNode>>& children) const
{
    using namespace DAVA;

    if (node->propertyType == PropertyPanel::SplineObject)
    {
        return;
    }

    ChildCreatorExtension::ExposeChildren(node, children);

    if (node->cachedValue.CanGet<SplineComponent*>())
    {
        Reflection::Field field = node->field;
        field.key = FastName("SplineEditor");
        std::shared_ptr<PropertyNode> splineEditorNode = allocator->CreatePropertyNode(node, std::move(field), static_cast<int32>(children.size()), PropertyPanel::SplineObject);
        children.push_back(splineEditorNode);
    }
}

//////////////////////////////////////////////////////////////////////////

std::unique_ptr<DAVA::BaseComponentValue> SplinePointEditorCreator::GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const
{
    using namespace DAVA;

    if (node->propertyType == PropertyPanel::SplinePointObject)
    {
        return std::make_unique<SplineEditorWidgetDetails::SplinePointEditorComponentValue>();
    }

    return EditorComponentExtension::GetEditor(node);
}

void SplinePointChildCreator::ExposeChildren(const std::shared_ptr<DAVA::PropertyNode>& node, DAVA::Vector<std::shared_ptr<DAVA::PropertyNode>>& children) const
{
    using namespace DAVA;

    if (node->propertyType == PropertyPanel::SplinePointObject)
    {
        return;
    }

    ChildCreatorExtension::ExposeChildren(node, children);

    if (node->cachedValue.CanGet<SplineComponent::SplinePoint*>())
    {
        Reflection::Field field = node->field;
        field.key = FastName("SplinePointEditor");
        std::shared_ptr<PropertyNode> splineEditorNode = allocator->CreatePropertyNode(node, std::move(field), static_cast<int32>(children.size()), PropertyPanel::SplinePointObject);
        children.push_back(splineEditorNode);
    }
}