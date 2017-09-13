#include "Modules/CreatingControlsModule/CreatingControlsSystem.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "UI/CommandExecutor.h"
#include "Utils/ControlPlacementUtils.h"

CreatingControlsSystem::CreatingControlsSystem(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui)
    : BaseEditorSystem(parent, accessor)
    , ui(ui)
{
    documentDataWrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<DocumentData>());
}

EditorSystemsManager::eDragState CreatingControlsSystem::RequireNewState(DAVA::UIEvent* currentInput)
{
    return createFromControl == nullptr ? EditorSystemsManager::NoDrag : EditorSystemsManager::AddingControl;
}

bool CreatingControlsSystem::CanProcessInput(DAVA::UIEvent* currentInput) const
{
    return (systemsManager->GetDragState() == EditorSystemsManager::AddingControl);
}

void CreatingControlsSystem::ProcessInput(DAVA::UIEvent* currentInput)
{
    using namespace DAVA;

    if (currentInput->device == eInputDevices::MOUSE && currentInput->phase == UIEvent::Phase::ENDED)
    {
        AddControlAtPoint(currentInput->point);
    }
    else if (currentInput->device == eInputDevices::KEYBOARD && currentInput->key == eInputElements::KB_ESCAPE)
    {
        ClearAddingTask();
    }
}

void CreatingControlsSystem::OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState)
{
    if (previousState == EditorSystemsManager::AddingControl)
    {
        ClearAddingTask();
    }
}

bool CreatingControlsSystem::IsDependsOnCurrentPackage(ControlNode* control) const
{
    using namespace DAVA::TArc;

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return false;
    }
    DocumentData* documentData = activeContext->GetData<DocumentData>();
    if (documentData == nullptr)
    {
        return false;
    }

    PackageNode* currentPackage = documentData->GetPackageNode();
    if (currentPackage != nullptr)
    {
        return false;
    }

    return control->IsDependsOnPackage(currentPackage);
}

void CreatingControlsSystem::SetCreateByClick(ControlNode* control)
{
    createFromControl = control;
    controlDependsOnPackage = (createFromControl == nullptr ? false : IsDependsOnCurrentPackage(createFromControl));
}

void CreatingControlsSystem::OnPackageChanged()
{
    using namespace DAVA::TArc;
    if (controlDependsOnPackage)
    {
        ClearAddingTask();
    }
}

void CreatingControlsSystem::AddControlAtPoint(const DAVA::Vector2& point)
{
    DAVA::TArc::DataContext* active = accessor->GetActiveContext();
    DVASSERT(active != nullptr);
    DocumentData* docData = active->GetData<DocumentData>();
    if (docData != nullptr)
    {
        PackageNode* package = docData->GetPackageNode();
        DVASSERT(package != nullptr);

        uint32 destIndex = 0;
        PackageBaseNode* destNode = systemsManager->GetControlNodeAtPoint(point);
        if (destNode == nullptr)
        {
            destNode = DynamicTypeCheck<PackageBaseNode*>(package->GetPackageControlsNode());
            destIndex = systemsManager->GetIndexOfNearestRootControl(point);
        }
        else
        {
            destIndex = destNode->GetCount();
        }

        ControlsContainerNode* destControlContainer = dynamic_cast<ControlsContainerNode*>(destNode);
        if (destControlContainer != nullptr)
        {
            docData->BeginBatch("Copy control from library");
            CommandExecutor executor(accessor, ui);

            Vector<ControlNode*> newNodes;
            newNodes = executor.CopyControls({ createFromControl }, destControlContainer, destIndex);
            if (destNode != package->GetPackageControlsNode())
            {
                ControlNode* destControl = dynamic_cast<ControlNode*>(destNode);
                if (destControl != nullptr && newNodes.size() == 1)
                {
                    ControlNode* newNode = newNodes.front();
                    ControlPlacementUtils::SetAbsoulutePosToControlNode(package, newNode, destControl, point);
                    AbstractProperty* postionProperty = newNode->GetRootProperty()->FindPropertyByName("position");
                    AbstractProperty* sizeProperty = newNode->GetRootProperty()->FindPropertyByName("size");
                    newNode->GetRootProperty()->SetProperty(postionProperty, Any(newNode->GetControl()->GetPosition()));
                    newNode->GetRootProperty()->SetProperty(sizeProperty, Any(newNode->GetControl()->GetSize()));

                    SelectedNodes newSelection = { newNode };
                    documentDataWrapper.SetFieldValue(DocumentData::selectionPropertyName, newSelection);
                }
            }

            docData->EndBatch();
        }
    }

    ClearAddingTask();
}

void CreatingControlsSystem::ClearAddingTask()
{
    createFromControl = nullptr;
}
