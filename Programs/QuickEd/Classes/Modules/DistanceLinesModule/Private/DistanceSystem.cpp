#include "Classes/Modules/DistanceLinesModule/Private/DistanceSystem.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLines.h"
#include "Classes/Modules/DistanceLinesModule/Private/DistanceLinesFactory.h"

#include "Classes/Modules/DocumentsModule/EditorSystemsData.h"
#include "Modules/DocumentsModule/DocumentData.h"
#include "Modules/CanvasModule/CanvasData.h"
#include "Modules/UpdateViewsSystemModule/UpdateViewsSystem.h"
#include "EditorSystems/UIControlUtils.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/Utils/Utils.h>

#include <Math/Vector.h>
#include <UI/UIControlSystem.h>
#include <UI/UIControl.h>
#include <UI/UIStaticText.h>
#include <Render/2D/FTFont.h>
#include <Utils/UTF8Utils.h>

DistanceSystem::DistanceSystem(EditorSystemsManager* parent, DAVA::TArc::ContextAccessor* accessor)
    : BaseEditorSystem(accessor)
    , canvasDataAdapter(accessor)
    , font(nullptr)
{
    using namespace DAVA;

    FilePath fntPath = FilePath("~res:/QuickEd/Fonts/DejaVuSans.ttf");
    font.Set(FTFont::Create(fntPath));
    font->SetSize(10.0f);
}

DistanceSystem::~DistanceSystem() = default;

CanvasControls DistanceSystem::CreateCanvasControls()
{
    using namespace DAVA;

    DVASSERT(canvas.Valid() == false);
    canvas.Set(new UIControl());
    canvas->SetName("Distance_system_canvas");

    return { { canvas } };
}

void DistanceSystem::DeleteCanvasControls(const CanvasControls& canvasControls)
{
    canvas = nullptr;
}

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

    EditorSystemsData* systemsData = activeContext->GetData<EditorSystemsData>();
    ControlNode* highlightedNode = systemsData->GetHighlightedNode();
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

    if (highlightedNode != parent && highlightedNode->GetParent() != parent)
    {
        return false;
    }

    if (selectedControls.size() == 1 && selectedControls.find(highlightedNode) != selectedControls.end())
    {
        return false;
    }

    return true;
}

void DistanceSystem::OnUpdate()
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    canvas->RemoveAllControls();
    if (CanDrawDistances() == false)
    {
        return;
    }

    //prepare data
    DataContext* activeContext = accessor->GetActiveContext();
    EditorSystemsData* systemsData = activeContext->GetData<EditorSystemsData>();
    ControlNode* highlightedNode = systemsData->GetHighlightedNode();
    UIControl* highlightedControl = highlightedNode->GetControl();

    DocumentData* documentData = activeContext->GetData<DocumentData>();
    Set<ControlNode*> selectedControls = documentData->GetSelectedControls();

    ControlNode* firstSelected = *selectedControls.begin();
    UIControl* selectionParent = firstSelected->GetControl()->GetParent();
    ControlNode* selectedNode = *selectedControls.begin();
    UIControl* selectedControl = selectedNode->GetControl();

    ControlsLinesFactory::ControlLinesFactoryParams params;
    params.accessor = accessor;
    params.font = font;
    params.selectedControl = selectedControl;
    params.highlightedControl = highlightedControl;
    std::unique_ptr<LinesFactory> factory(new ControlsLinesFactory(params));
    Vector<std::unique_ptr<DistanceLine>> lines = factory->CreateLines();

    for (const std::unique_ptr<DistanceLine>& line : lines)
    {
        line->Draw(canvas);
    }
}
