#include "LibraryControl.h"


LibraryControl::LibraryControl(const Rect & rect)
    :   UIControl(rect)
{
    folderPath = "/";
    
    fontLight = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    fontLight->SetSize(12);
    fontLight->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

    fontDark = FTFont::Create("~res:/Fonts/MyriadPro-Regular.otf");
    fontDark->SetSize(12);
    fontDark->SetColor(Color(0.1f, 0.1f, 0.1f, 1.0f));

    
    GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
    
    filesTree = new UIHierarchy(Rect(0, BUTTON_HEIGHT, rect.dx, rect.dy - BUTTON_HEIGHT));
    filesTree->SetCellHeight(CELL_HEIGHT);
    filesTree->SetDelegate(this);
    filesTree->SetClipContents(true);
    filesTree->GetBackground()->SetDrawType(UIControlBackground::DRAW_FILL);
    filesTree->GetBackground()->SetColor(Color(0.92f, 0.92f, 0.92f, 1.0f));
    AddControl(filesTree);
    
    //button
    refreshButton = new UIButton(Rect(0, 0, rect.dx, BUTTON_HEIGHT));
    refreshButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    refreshButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    refreshButton->GetStateBackground(UIControl::STATE_NORMAL)->SetColor(Color(0.0f, 0.0f, 0.0f, 0.5f));
    refreshButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));
    refreshButton->SetStateFont(UIControl::STATE_NORMAL, fontLight);
    refreshButton->SetStateFont(UIControl::STATE_PRESSED_INSIDE, fontLight);
    refreshButton->SetStateText(UIControl::STATE_NORMAL, L"Refresh Library");
    refreshButton->SetStateText(UIControl::STATE_PRESSED_INSIDE, L"Refresh Library");
    AddControl(refreshButton);
}
    
LibraryControl::~LibraryControl()
{
    SafeRelease(refreshButton);
    
    SafeRelease(filesTree);
    
    SafeRelease(fontLight);
    SafeRelease(fontDark);
}

bool LibraryControl::IsNodeExpandable(UIHierarchy *forHierarchy, void *forNode)
{
    if(forHierarchy == filesTree)
    {

    }
    
    return false;
}

int32 LibraryControl::ChildrenCount(UIHierarchy *forHierarchy, void *forParent)
{
    if(forHierarchy == filesTree)
    {
    }
    
    return 0;
}

void * LibraryControl::ChildAtIndex(UIHierarchy *forHierarchy, void *forParent, int32 index)
{
    if(forHierarchy == filesTree)
    {
    }
    
    return NULL;
}

UIHierarchyCell * LibraryControl::CellForNode(UIHierarchy *forHierarchy, void *node)
{
    UIHierarchyCell *c = NULL;
    if(forHierarchy == filesTree)
    {
        c = forHierarchy->GetReusableCell("Library cell"); //try to get cell from the reusable cells store
        if(!c)
        { //if cell of requested type isn't find in the store create new cell
            Rect r = GetRect();
            c = new UIHierarchyCell(Rect(0, 0, r.dx, CELL_HEIGHT), "Library cell");
        }
        
        c->text->SetText(L"Cell");
    }
    
    c->text->SetFont(fontDark);
    c->text->SetAlign(ALIGN_LEFT|ALIGN_VCENTER);

    Color color(0.1f, 0.5f, 0.05f, 1.0f);
    c->openButton->SetStateDrawType(UIControl::STATE_NORMAL, UIControlBackground::DRAW_FILL);
    c->openButton->SetStateDrawType(UIControl::STATE_PRESSED_INSIDE, UIControlBackground::DRAW_FILL);
    c->openButton->SetStateDrawType(UIControl::STATE_HOVER, UIControlBackground::DRAW_FILL);
    c->openButton->GetStateBackground(UIControl::STATE_NORMAL)->color = color;
    c->openButton->GetStateBackground(UIControl::STATE_HOVER)->color = color + 0.1f;
    c->openButton->GetStateBackground(UIControl::STATE_PRESSED_INSIDE)->color = color + 0.3f;

    return c;
}

void LibraryControl::OnCellSelected(UIHierarchy *forHierarchy, UIHierarchyCell *selectedCell)
{
    if(forHierarchy == filesTree)
    {
    }
}


void LibraryControl::Update(float32 timeElapsed)
{
    UIControl::Update(timeElapsed);
}

void LibraryControl::WillAppear()
{
    filesTree->Refresh();
}

void LibraryControl::SetPath(const String &path)
{
    folderPath = path;
    
    if(GetParent())
    {
        filesTree->Refresh();
    }
}
