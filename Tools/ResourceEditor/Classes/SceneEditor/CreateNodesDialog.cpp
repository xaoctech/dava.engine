#include "CreateNodesDialog.h"
#include "ControlsFactory.h"

#include "NodesPropertyControl.h"
#include "PropertyControlCreator.h"
#include "../EditorScene.h"
#include "SceneEditorScreenMain.h"
#include "../AppScreens.h"
#include "EditorBodyControl.h"
#include "EditorLightNode.h"
#include "Scene3D/UserNode.h"
#include "EditorSettings.h"

CreateNodesDialog::CreateNodesDialog(const Rect & rect)
    :   DraggableDialog(rect)
{
    ControlsFactory::CustomizeDialog(this);

    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
	propertyList = 0;
    
    header = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFontLight());
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    AddControl(header);
    
    int32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    int32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH * 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnCancel));
    AddControl(btnCancel);
    
    buttonX += ControlsFactory::BUTTON_WIDTH;
    UIButton *btnOk = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.ok"));
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnOk));
    AddControl(btnOk);
    
    propertyRect = Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, buttonY - ControlsFactory::BUTTON_HEIGHT);

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

void CreateNodesDialog::OnCancel(BaseObject *, void *, void *)
{
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_CANCEL);
    }
}

void CreateNodesDialog::OnOk(BaseObject *, void *, void *)
{    
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
    propertyList->SetWorkingScene(scene);
}

void CreateNodesDialog::SetHeader(const WideString &headerText)
{
    header->SetText(headerText);
}

void CreateNodesDialog::CreateNode(ResourceEditor::eNodeType nodeType)
{
    SafeRelease(sceneNode);

	SceneEditorScreenMain * screen = (SceneEditorScreenMain*)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
	EditorScene * editorScene = screen->FindCurrentBody()->bodyControl->GetScene();
	scene = editorScene;

    switch (nodeType) 
    {
        case ResourceEditor::NODE_LANDSCAPE:
            SetHeader(LocalizedString(L"createnode.landscape"));
            sceneNode = new LandscapeNode();
            sceneNode->SetName("Landscape");
            break;

        case ResourceEditor::NODE_LIGHT:
        {
            SetHeader(LocalizedString(L"createnode.light"));
            
            sceneNode = EditorLightNode::CreateSceneAndEditorLight();
            sceneNode->SetName("Light");
            break;
        }

        case ResourceEditor::NODE_SERVICE_NODE:
        {
            SetHeader(LocalizedString(L"createnode.servicenode"));
            sceneNode = new SceneNode();
            KeyedArchive *customProperties = sceneNode->GetCustomProperties();
            customProperties->SetBool("editor.isLocked", true);
            sceneNode->SetName("Servicenode");
            break;
        }

        case ResourceEditor::NODE_BOX:
            SetHeader(LocalizedString(L"createnode.box"));
            sceneNode = new CubeNode();
            sceneNode->SetName("Cube");
            break;

        case ResourceEditor::NODE_SPHERE:
            SetHeader(LocalizedString(L"createnode.sphere"));
            sceneNode = new SphereNode();
            sceneNode->SetName("Sphere");
            break;

        case ResourceEditor::NODE_CAMERA:
            SetHeader(LocalizedString(L"createnode.camera"));
            sceneNode = new Camera();
            ((Camera *)sceneNode)->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            sceneNode->SetName("Camera");
            break;

		case ResourceEditor::NODE_IMPOSTER:
			SetHeader(LocalizedString(L"createnode.imposter"));
			sceneNode = new ImposterNode();
			sceneNode->SetName("Imposter");
			break;

		case ResourceEditor::NODE_PARTICLE_EMITTER:
		{
			SetHeader(LocalizedString(L"createnode.particleemitter"));
			ParticleEmitterNode * node = new ParticleEmitterNode();
			node->LoadFromYaml("~res:/Particles/sparkles.yaml");

			sceneNode = node;
		}
			break;

		case ResourceEditor::NODE_USER_NODE:
			SetHeader(LocalizedString(L"createnode.usernode"));
			sceneNode = new UserNode();
			sceneNode->SetName("UserNode");
			break;

            
        default:
            break;
    }

	propertyList = PropertyControlCreator::Instance()->CreateControlForNode(sceneNode, propertyRect, true);
    SafeRetain(propertyList);
	AddControl(propertyList);

	SetScene(editorScene);
    
    propertyList->ReadFrom(sceneNode);
}

void CreateNodesDialog::WillDisappear()
{
	RemoveControl(propertyList);
	SafeRelease(propertyList);
}

