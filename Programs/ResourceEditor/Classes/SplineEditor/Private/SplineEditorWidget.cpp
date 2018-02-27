#include "Classes/SplineEditor/Private/SplineEditorWidget.h"

#include "Classes/PropertyPanel/PropertyPanelCommon.h"
#include "Classes/SplineEditor/Private/SplineEditorPPExtensions.h"

#include <REPlatform/DataNodes/SceneData.h>
#include <REPlatform/Scene/Components/SplineEditorDrawComponent.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/SplineEditorSystem.h>

#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/QtBoxLayouts.h>

#include <Base/Any.h>
#include <Functional/Function.h>
#include <Scene3D/Components/SplineComponent.h>

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

        Reflection ref = Reflection::Create(drawComponent);

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
