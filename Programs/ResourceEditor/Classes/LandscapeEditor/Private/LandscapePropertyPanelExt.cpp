#include "Classes/LandscapeEditor/Private/LandscapePropertyPanelExt.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseLandscapeTool.h"
#include "REPlatform/DataNodes/SceneData.h"

#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/Widget.h>
#include <TArc/Controls/QtFlowLayout.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/QtConnections.h>

#include <Base/Any.h>
#include <Functional/Function.h>
#include <Render/Highlevel/RenderObject.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/LandscapeComponent.h>

#include <QHBoxLayout>
#include <QToolButton>

namespace LandscapePropertyPanelExtDetails
{
using namespace DAVA;

class LandscapeEditorComponentValue : public DAVA::BaseComponentValue
{
public:
    LandscapeEditorComponentValue() = default;
    ~LandscapeEditorComponentValue()
    {
        delete toolWidget;
    }

    bool RepaintOnUpdateRequire() const override
    {
        return true;
    }

    bool IsSpannedControl() const override
    {
        return true;
    }

protected:
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
        buttons.clear();

        Widget* container = new Widget(parent);
        QWidget* containerWidget = container->ToWidgetCast();
        QVBoxLayout* mainLayout = new QVBoxLayout(containerWidget);
        mainLayout->setSpacing(2);
        mainLayout->setMargin(2);

        QtFlowLayout* layout = new QtFlowLayout(1, 2, 2);
        layout->setSpacing(2);
        layout->setMargin(1);
        mainLayout->addLayout(layout);

        DataContext* ctx = GetAccessor()->GetActiveContext();
        DVASSERT(ctx != nullptr);
        SceneData* data = ctx->GetData<SceneData>();
        LandscapeEditorSystemV2* system = data->GetScene()->GetSystem<LandscapeEditorSystemV2>();
        const BaseLandscapeTool* activeTool = system->GetActiveTool();

        for (BaseLandscapeTool* tool : system->GetAvailableTools())
        {
            BaseLandscapeTool::ButtonInfo info = tool->GetButtonInfo();
            QToolButton* button = new QToolButton(containerWidget);
            button->setAutoRaise(false);
            button->setCheckable(true);
            button->setChecked(tool == activeTool);
            button->setToolTip(info.tooltip);
            button->setIcon(info.icon);
            button->setIconSize(QSize(24, 24));
            connections.AddConnection(button, &QAbstractButton::toggled, DAVA::Bind(&LandscapeEditorComponentValue::OnButtonToggled, this, DAVA::_1, button, tool));
            buttons.push_back(button);

            layout->addWidget(button);
        };

        if (activeTool != nullptr)
        {
            InjectWidget(const_cast<BaseLandscapeTool*>(activeTool), mainLayout);
            repaintASAP.Emit();
        }

        return container;
    }

    void OnButtonToggled(bool checked, QToolButton* toggledButton, BaseLandscapeTool* tool)
    {
        SCOPED_VALUE_GUARD(bool, inToggle, true, void());

        for (QPointer<QToolButton> button : buttons)
        {
            if (button.isNull() == false && button != toggledButton)
            {
                button->setChecked(false);
            }
        }

        ContextAccessor* accessor = GetAccessor();
        DataContext* ctx = accessor->GetActiveContext();
        DVASSERT(ctx != nullptr);
        SceneData* data = ctx->GetData<SceneData>();
        LandscapeEditorSystemV2* system = data->GetScene()->GetSystem<LandscapeEditorSystemV2>();

        delete toolWidget;
        toolWidget = nullptr;
        if (system->GetActiveTool() != nullptr)
        {
            system->DeactivateTool();
        }

        if (checked == true)
        {
            tool->SetAccessor(accessor);
            system->ActivateTool(tool);
            InjectWidget(tool, realWidget->layout());
        }

        repaintASAP.Emit();
    }

    void InjectWidget(BaseLandscapeTool* tool, QLayout* layout)
    {
        BaseLandscapeTool::WidgetParams params(GetWindowKey());
        params.accessor = GetAccessor();
        params.ui = GetUI();
        params.processor = GetDataProcessor(true);
        params.parent = realWidget;
        toolWidget = tool->CreateEditorWidget(params);
        toolWidget->setObjectName("toolWidget");
        layout->addWidget(toolWidget);
        toolWidget->show();
    }

    QWidget* toolWidget = nullptr;
    bool inToggle = false;
    DAVA::Vector<QPointer<QToolButton>> buttons;
    QtConnections connections;
};
} // namespace LandscapePropertyPanelExtDetails

std::unique_ptr<DAVA::BaseComponentValue> LandscapeEditorCreator::GetEditor(const std::shared_ptr<const DAVA::PropertyNode>& node) const
{
    using namespace DAVA;

    if (node->propertyType == PropertyPanel::LandscapeObject)
    {
        return std::make_unique<LandscapePropertyPanelExtDetails::LandscapeEditorComponentValue>();
    }

    return EditorComponentExtension::GetEditor(node);
}

void LandscapeEditorChildCreator::ExposeChildren(const std::shared_ptr<DAVA::PropertyNode>& node, DAVA::Vector<std::shared_ptr<DAVA::PropertyNode>>& children) const
{
    using namespace DAVA;

    if (node->propertyType == PropertyPanel::LandscapeObject)
    {
        return;
    }

    ChildCreatorExtension::ExposeChildren(node, children);

    bool addLandscapeEditorPanel = node->cachedValue.CanGet<LandscapeComponent*>();
    if (node->cachedValue.CanGet<RenderComponent*>())
    {
        RenderComponent* rc = node->cachedValue.Get<RenderComponent*>();
        addLandscapeEditorPanel |= (rc->GetRenderObject()->GetType() == RenderObject::TYPE_LANDSCAPE);
    }

    if (addLandscapeEditorPanel)
    {
        Reflection::Field f = node->field;
        f.key = FastName("LandscapeEditor");
        std::shared_ptr<PropertyNode> landscape = allocator->CreatePropertyNode(node, std::move(f), static_cast<int32>(children.size()), PropertyPanel::LandscapeObject);
        children.push_back(landscape);
    }
}
