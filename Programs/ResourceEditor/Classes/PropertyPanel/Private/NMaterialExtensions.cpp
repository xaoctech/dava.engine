#include "Classes/PropertyPanel/NMaterialExtensions.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"

#include <REPlatform/Commands/MaterialAssignCommand.h>
#include <REPlatform/DataNodes/SelectionData.h>
#include <REPlatform/Global/GlobalOperations.h>

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Core/OperationInvoker.h>
#include <TArc/DataProcessing/DataContext.h>
#include <TArc/Utils/Utils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Render/Material/NMaterial.h>
#include <Base/Result.h>

namespace NMaterialExtensionsDetail
{
using namespace DAVA;

//////////////////////////////////////////////////////////////////////////

class OpenMaterialEditorProducer : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::PropertyNode>& node) const override;
    Info GetInfo() const override;
    bool OnlyForSingleSelection() const override;
    std::unique_ptr<Command> CreateCommand(const std::shared_ptr<DAVA::PropertyNode>& node, const Params& params) const override;
};

bool OpenMaterialEditorProducer::IsApplyable(const std::shared_ptr<DAVA::PropertyNode>& node) const
{
    return node->field.ref.GetValueType() == Type::Instance<NMaterial*>();
}

M::CommandProducer::Info OpenMaterialEditorProducer::GetInfo() const
{
    Info info;
    info.icon = DAVA::SharedIcon(":/QtIcons/3d.png");
    info.tooltip = "Edit material";
    return info;
}

bool OpenMaterialEditorProducer::OnlyForSingleSelection() const
{
    return true;
}

std::unique_ptr<Command> OpenMaterialEditorProducer::CreateCommand(const std::shared_ptr<DAVA::PropertyNode>& node, const Params& params) const
{
    NMaterial* material = node->field.ref.GetValue().Get<NMaterial*>();
    params.invoker->Invoke(DAVA::ShowMaterial.ID, material);
    return std::unique_ptr<Command>();
}

//////////////////////////////////////////////////////////////////////////

class AssignMaterialProducer : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::PropertyNode>& node) const override;
    Info GetInfo() const override;
    bool OnlyForSingleSelection() const override;
    void CreateCache(DAVA::ContextAccessor* accessor) override;
    void ClearCache() override;
    std::unique_ptr<Command> CreateCommand(const std::shared_ptr<DAVA::PropertyNode>& node, const Params& params) const override;

protected:
    Entity* entity = nullptr;
};

bool AssignMaterialProducer::IsApplyable(const std::shared_ptr<DAVA::PropertyNode>& node) const
{
    return (node->field.ref.GetValueType() == Type::Instance<DAVA::NMaterial*>()) && (node->field.ref.GetMeta<M::ReadOnly>() == nullptr);
}

M::CommandProducer::Info AssignMaterialProducer::GetInfo() const
{
    Info info;
    info.icon = DAVA::SharedIcon(":/QtIcons/sphere_arrow.png");
    info.tooltip = "Assign Selected material";
    return info;
}

bool AssignMaterialProducer::OnlyForSingleSelection() const
{
    return true;
}

void AssignMaterialProducer::CreateCache(DAVA::ContextAccessor* accessor)
{
    DAVA::SelectionData* selection = accessor->GetActiveContext()->GetData<DAVA::SelectionData>();
    DVASSERT(selection->GetSelection().GetSize() == 1u);

    entity = selection->GetSelection().GetFirst().AsEntity();
}

void AssignMaterialProducer::ClearCache()
{
    entity = nullptr;
}

std::unique_ptr<Command> AssignMaterialProducer::CreateCommand(const std::shared_ptr<DAVA::PropertyNode>& node, const Params& params) const
{
    MaterialEditor* editor = MaterialEditor::Instance();
    DAVA::Set<DAVA::NMaterial*> selectedMaterials = editor->GetSelection();

    bool showNotification = false;
    DAVA::NotificationParams notificationParams;
    notificationParams.title = "Material Assign Error";
    notificationParams.message.type = DAVA::Result::RESULT_ERROR;

    if (selectedMaterials.size() == 0)
    {
        notificationParams.message.message = "You should select material in MaterialEditor to assign it";
        showNotification = true;
    }
    else if (selectedMaterials.size() > 1)
    {
        notificationParams.message.message = "You should select single material in MaterialEditor to assign it";
        showNotification = true;
    }
    else //size() == 1
    {
        DAVA::NMaterial* material = *selectedMaterials.begin();
        const DAVA::M::MaterialType* materialType = node->field.ref.GetMeta<DAVA::M::MaterialType>();

        if (materialType != nullptr && materialType->type != material->GetMaterialType())
        {
            const EnumMap* enumMap = GlobalEnumMap<DAVA::NMaterial::eType>::Instance();
            notificationParams.message.message = DAVA::Format("Selected material has invalid type (%s). %s is needed",
                                                              enumMap->ToString(material->GetMaterialType()), enumMap->ToString(materialType->type));
            showNotification = true;
        }
        else
        {
            return std::unique_ptr<DAVA::MaterialAssignCommand>(new DAVA::MaterialAssignCommand(entity, material, node->field));
        }
    }

    params.ui->ShowNotification(DAVA::mainWindowKey, notificationParams);
    return std::unique_ptr<Command>();
}
}

//////////////////////////////////////////////////////////////////////////

DAVA::M::CommandProducerHolder CreateNMaterialCommandProducer()
{
    using namespace NMaterialExtensionsDetail;
    DAVA::M::CommandProducerHolder holder;
    holder.AddCommandProducer(std::make_unique<OpenMaterialEditorProducer>());
    holder.AddCommandProducer(std::make_unique<AssignMaterialProducer>());
    return holder;
}
