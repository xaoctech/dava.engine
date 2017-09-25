#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLines.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesFactory.h"

#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"
#include "Classes/Modules/DocumentsModule/DocumentData.h"
#include "Classes/Modules/CanvasModule/CanvasData.h"
#include "Classes/Modules/UpdateViewsSystemModule/UpdateViewsSystem.h"
#include "Classes/EditorSystems/UIControlUtils.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/Utils.h>

#include <Math/Vector.h>
#include <UI/UIControl.h>
#include <Render/2D/FTFont.h>

DistanceSystem::DistanceSystem(DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , canvasDataAdapter(accessor)
    , factory(new DistanceLinesFactory())
{
}

DistanceSystem::~DistanceSystem() = default;

BaseEditorSystem::eSystems DistanceSystem::GetOrder() const
{
    return DISTANCE_LINES;
}

bool DistanceSystem::CanDrawDistances() const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (IsKeyPressed(eModifierKeys::ALT) == false)
    {
        return false;
    }

    DataContext* activeContext = accessor->GetActiveContext();
    if (activeContext == nullptr)
    {
        return false;
    }

    DVASSERT(getHighlight != nullptr);

    ControlNode* highlightedNode = getHighlight();
    if (highlightedNode == nullptr)
    {
        return false;
    }

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    Set<ControlNode*> selectedControls = documentData->GetSelectedControls();
    if (selectedControls.size() != 1)
    {
        return false;
    }

    PackageBaseNode* parent = (*selectedControls.begin())->GetParent();

    if (selectedControls.find(highlightedNode) != selectedControls.end())
    {
        return false;
    }

    return true;
}

void DistanceSystem::OnUpdate()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (CanDrawDistances() == false)
    {
        return;
    }

    //prepare data
    DataContext* activeContext = accessor->GetActiveContext();
    EditorSystemsData* systemsData = activeContext->GetData<EditorSystemsData>();

    DVASSERT(getHighlight != nullptr);
    ControlNode* highlightedNode = getHighlight();
    UIControl* highlightedControl = highlightedNode->GetControl();

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    Set<ControlNode*> selectedControls = documentData->GetSelectedControls();

    ControlNode* firstSelected = *selectedControls.begin();
    UIControl* selectionParent = firstSelected->GetControl()->GetParent();
    ControlNode* selectedNode = *selectedControls.begin();
    UIControl* selectedControl = selectedNode->GetControl();

    DistanceLinesFactory::Params params(selectedControl, highlightedControl);
    params.accessor = accessor;
    params.painter = GetPainter();
    params.order = GetOrder();
    Vector<std::unique_ptr<DistanceLine>> lines = factory->CreateLines(params);

    for (const std::unique_ptr<DistanceLine>& line : lines)
    {
        line->Draw();
    }
}
