#include "CreateNodesDialog.h"
#include "ControlsFactory.h"

#include "NodesPropertyControl.h"
#include "SceneNodeIDs.h"

CreateNodesDialog::CreateNodesDialog(const Rect & rect)
    :   DraggableDialog(rect)
{
    ControlsFactory::CustomizeDialog(this);

    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
    
    header = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFontLight());
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    AddControl(header);
    
    int32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT - 2;
    int32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH * 2 - 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), L"Cancel");
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnCancel));
    AddControl(btnCancel);
    
    buttonX += ControlsFactory::BUTTON_WIDTH + 1;
    UIButton *btnOk = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), L"Ok");
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnOk));
    AddControl(btnOk);
    
    Rect propertyRect = Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, buttonY - ControlsFactory::BUTTON_HEIGHT);
    propertyList = new NodesPropertyControl(propertyRect, true);
    AddControl(propertyList);

    SafeRelease(btnCancel);
    SafeRelease(btnOk);
}
    
CreateNodesDialog::~CreateNodesDialog()
{
    SafeRelease(sceneNode);

    SafeRelease(header);
    SafeRelease(propertyList);
    dialogDelegate = NULL;
}

void CreateNodesDialog::SetDelegate(CreateNodesDialogDelegeate *delegate)
{
    dialogDelegate = delegate;
}

void CreateNodesDialog::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_CANCEL);
    }
}

void CreateNodesDialog::OnOk(BaseObject * object, void * userData, void * callerData)
{    
    propertyList->WriteTo(sceneNode);
    
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_OK);
    }
}

SceneNode * CreateNodesDialog::GetSceneNode()
{
    return sceneNode;
}


void CreateNodesDialog::SetScene(Scene *_scene)
{
    scene = _scene;
}

void CreateNodesDialog::SetHeader(const WideString &headerText)
{
    header->SetText(headerText);
}

void CreateNodesDialog::CreateNode(int32 nodeID)
{
    SafeRelease(sceneNode);

    switch (nodeID) 
    {
        case ECNID_LANDSCAPE:
            SetHeader(L"Create landscape node");
            sceneNode = new LandscapeNode(scene);
            sceneNode->SetName("Landscape");
            break;

        case ECNID_LIGHT:
            SetHeader(L"Create light node");
            sceneNode = new LightNode(scene);
            sceneNode->SetName("Light");
            break;

        case ECNID_SERVICENODE:
            SetHeader(L"Create service node");
            sceneNode = new SceneNode(scene);
            sceneNode->SetName("Servicenode");
            break;

        case ECNID_BOX:
            SetHeader(L"Create box node");
            sceneNode = new CubeNode(scene);
            sceneNode->SetName("Cube");
            break;

        case ECNID_SPHERE:
            SetHeader(L"Create sphere node");
            sceneNode = new SphereNode(scene);
            sceneNode->SetName("Sphere");
            break;

        case ECNID_CAMERA:
            SetHeader(L"Create camera");
            sceneNode = new Camera(scene);
            sceneNode->SetName("Camera");
            break;
            
        default:
            break;
    }
    
    propertyList->ReadFrom(sceneNode);
}

