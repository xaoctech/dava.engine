#include "TArc/SharedModules/SettingsModule/Private/SettingsDialog.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/TimerUpdater.h"
#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/SimpleModifyExtension.h"
#include "TArc/Controls/ReflectedButton.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Utils/ReflectionHelpers.h"
#include "TArc/DataProcessing/SettingsNode.h"

#include <Reflection/ReflectionRegistrator.h>

#include <QBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QDialogButtonBox>

namespace DAVA
{
namespace SettingsDialogDetails
{
const String settingsDialogPropertiesNode = "SettingsDialog";

class ExposeSettingsNodeExt : public ChildCreatorExtension
{
public:
    void ExposeChildren(const std::shared_ptr<PropertyNode>& parent, Vector<std::shared_ptr<PropertyNode>>& children) const override
    {
        if (parent->propertyType == PropertyNode::SelfRoot)
        {
            Vector<Reflection::Field> fields = parent->field.ref.GetFields();
            DVASSERT(fields.size() == 1);

            ForEachField(fields[0].ref, [&](Reflection::Field&& field) {
                if (CanBeExposed(field) == false)
                {
                    return;
                }

                const M::DisplayName* displayNameMeta = field.ref.GetMeta<M::DisplayName>();
                if (displayNameMeta == nullptr)
                {
                    displayNameMeta = GetTypeMeta<M::DisplayName>(field.ref.GetValue());
                }

                if (displayNameMeta != nullptr)
                {
                    field.key = displayNameMeta->displayName;
                }
                else
                {
                    const ReflectedType* type = GetValueReflectedType(field.ref);
                    field.key = type->GetPermanentName();
                }
                children.push_back(allocator->CreatePropertyNode(parent, std::move(field), static_cast<int32>(children.size()), PropertyNode::RealProperty));
            });
        }
        else
        {
            ChildCreatorExtension::ExposeChildren(parent, children);
        }
    }
};

class SettingsEditorCreatorExt : public EditorComponentExtension
{
public:
    std::unique_ptr<BaseComponentValue> GetEditor(const std::shared_ptr<const PropertyNode>& node) const
    {
        std::unique_ptr<BaseComponentValue> componentValue = EditorComponentExtension::GetEditor(node);

        if (node->propertyType == PropertyNode::GroupProperty ||
            (node->propertyType == PropertyNode::RealProperty &&
             node->cachedValue.GetType() == Type::Instance<SettingsNode*>()))
        {
            BaseComponentValue::Style style;
            style.fontBold = true;
            componentValue->SetStyle(style);
        }

        return std::unique_ptr<BaseComponentValue>(std::move(componentValue));
    }
};
} // namespace SettingsDialogDetails

SettingsDialog::SettingsDialog(const Params& params_, QWidget* parent)
    : QDialog(parent)
    , params(params_)
{
    setWindowFlags(Qt::Tool);
    setWindowTitle("Settings");

    updater.reset(new TimerUpdater(2500, TimerUpdater::DisableFastUpdate));

    QVBoxLayout* dlgLayout = new QVBoxLayout();
    dlgLayout->setMargin(5);

    PropertiesView::Params propertiesViewParams(DAVA::mainWindowKey);
    propertiesViewParams.accessor = params.accessor;
    propertiesViewParams.isInDevMode = false;
    propertiesViewParams.showToolBar = false;
    propertiesViewParams.ui = params.ui;
    propertiesViewParams.settingsNodeName = SettingsDialogDetails::settingsDialogPropertiesNode;
    propertiesViewParams.updater = updater;
    propertiesViewParams.objectsField = params.objectsField;
    view = new PropertiesView(propertiesViewParams);
    view->RegisterExtension(std::make_shared<SettingsDialogDetails::ExposeSettingsNodeExt>());
    view->RegisterExtension(std::make_shared<SettingsDialogDetails::SettingsEditorCreatorExt>());
    view->RegisterExtension(std::make_shared<SimpleModifyExtension>(std::weak_ptr<PropertiesView::Updater>(updater)));
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    dlgLayout->addWidget(view);

    QtHBoxLayout* buttonsLayout = new QtHBoxLayout();
    dlgLayout->addLayout(buttonsLayout);
    Reflection refModel = Reflection::Create(ReflectedObject(this));
    {
        ReflectedPushButton::Params buttonParams(params.accessor, params.ui, DAVA::mainWindowKey);
        buttonParams.fields[ReflectedPushButton::Fields::Text] = "resetToDefaultText";
        buttonParams.fields[ReflectedPushButton::Fields::Clicked] = "resetToDefault";
        ReflectedPushButton* button = new ReflectedPushButton(buttonParams, params.accessor, refModel, this);
        buttonsLayout->AddControl(button);
    }

    buttonsLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

    {
        ReflectedPushButton::Params buttonParams(params.accessor, params.ui, DAVA::mainWindowKey);
        buttonParams.fields[ReflectedPushButton::Fields::Text] = "acceptButtonText";
        buttonParams.fields[ReflectedPushButton::Fields::Clicked] = "accept";
        ReflectedPushButton* button = new ReflectedPushButton(buttonParams, params.accessor, refModel, this);
        buttonsLayout->AddControl(button);
    }

    setLayout(dlgLayout);
}

void SettingsDialog::OnResetPressed()
{
    ModalMessageParams dlgParams;
    dlgParams.icon = ModalMessageParams::Question;
    dlgParams.message = "Are you sure you want to reset settings to their default values?";
    dlgParams.title = "Reseting settings";
    dlgParams.buttons = ModalMessageParams::Buttons(ModalMessageParams::Yes | ModalMessageParams::No);
    dlgParams.defaultButton = ModalMessageParams::No;

    if (params.ui->ShowModalMessage(DAVA::mainWindowKey, dlgParams) == ModalMessageParams::Yes)
    {
        resetSettings.Emit();
    }
}

DAVA_REFLECTION_IMPL(SettingsDialog)
{
    ReflectionRegistrator<SettingsDialog>::Begin()
    .Method("resetToDefault", &SettingsDialog::OnResetPressed)
    .Method("accept", &SettingsDialog::accept)
    .Field("resetToDefaultText", [](SettingsDialog*) { return "Defaults"; }, nullptr)
    .Field("acceptButtonText", [](SettingsDialog*) { return "Ok"; }, nullptr)
    .End();
}
} // namespace DAVA
