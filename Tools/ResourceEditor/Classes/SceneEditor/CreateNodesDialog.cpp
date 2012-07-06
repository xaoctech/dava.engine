#include "CreateNodesDialog.h"
#include "ControlsFactory.h"

#include "NodesPropertyControl.h"
#include "PropertyControlCreator.h"
#include "SceneNodeIDs.h"
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

void CreateNodesDialog::OnCancel(BaseObject * object, void * userData, void * callerData)
{
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_CANCEL);
    }
}

void CreateNodesDialog::OnOk(BaseObject * object, void * userData, void * callerData)
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

void CreateNodesDialog::CreateNode(int32 nodeID)
{
    SafeRelease(sceneNode);

	SceneEditorScreenMain * screen = (SceneEditorScreenMain*)UIScreenManager::Instance()->GetScreen(SCREEN_SCENE_EDITOR_MAIN);
	EditorScene * editorScene = screen->FindCurrentBody()->bodyControl->GetScene();
	scene = editorScene;

    switch (nodeID) 
    {
        case ECNID_LANDSCAPE:
            SetHeader(LocalizedString(L"createnode.landscape"));
            sceneNode = new LandscapeNode();
            sceneNode->SetName("Landscape");
            break;

        case ECNID_LIGHT:
        {
            SetHeader(LocalizedString(L"createnode.light"));
            
            sceneNode = EditorLightNode::CreateSceneAndEditorLight();
            sceneNode->SetName("Light");
            break;
        }

        case ECNID_SERVICENODE:
        {
            SetHeader(LocalizedString(L"createnode.servicenode"));
            sceneNode = new SceneNode();
            KeyedArchive *customProperties = sceneNode->GetCustomProperties();
            customProperties->SetBool("editor.isLocked", true);
            sceneNode->SetName("Servicenode");
            break;
        }

        case ECNID_BOX:
            SetHeader(LocalizedString(L"createnode.box"));
            sceneNode = new CubeNode();
            sceneNode->SetName("Cube");
            break;

        case ECNID_SPHERE:
            SetHeader(LocalizedString(L"createnode.sphere"));
            sceneNode = new SphereNode();
            sceneNode->SetName("Sphere");
            break;

        case ECNID_CAMERA:
            SetHeader(LocalizedString(L"createnode.camera"));
            sceneNode = new Camera();
            ((Camera *)sceneNode)->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            sceneNode->SetName("Camera");
            break;

		case ECNID_IMPOSTER:
			SetHeader(LocalizedString(L"createnode.imposter"));
			sceneNode = new ImposterNode();
			sceneNode->SetName("Imposter");
			break;

		case ECNID_PARTICLE_EMITTER:
		{
			SetHeader(LocalizedString(L"createnode.particleemitter"));
			ParticleEmitterNode * node = new ParticleEmitterNode();
			node->SetName("ParticleEmitter");
			ParticleEmitter3D * emitter = new ParticleEmitter3D();
			emitter->LoadFromYaml("~res:/Particles/sparkles.yaml");
			node->SetEmitter(emitter);
			SafeRelease(emitter);

			sceneNode = node;
		}
			break;

		case ECNID_USERNODE:
			SetHeader(LocalizedString(L"createnode.usernode"));
			sceneNode = new UserNode();
			sceneNode->SetName("UserNode");
			break;


//        case ECNID_LODNODE:
//			SetHeader(LocalizedString(L"createnode.lodnode"));
//			sceneNode = new LodNode(scene);
//            for(int32 iLayer = 0; iLayer < LodNode::MAX_LOD_LAYERS; ++iLayer)
//            {
//                ((LodNode *)sceneNode)->SetLodLayerDistance(iLayer, EditorSettings::Instance()->GetLodLayerDistance(iLayer));
//            }
//            
//			sceneNode->SetName("LodNode");
//			break;
            
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

