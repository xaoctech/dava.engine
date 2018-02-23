#include "VisualScriptEditor/VisualScriptEditorModule.h"
#include "VisualScriptEditor/VisualScriptEditorDialog.h"
#include "VisualScriptEditor/Private/VisualScriptEditorDialogSettings.h"
#include "VisualScriptEditor/Private/VisualScriptEditorData.h"

#include <TArc/Utils/ModuleCollection.h>
#include <TArc/WindowSubSystem/QtAction.h>
#include <TArc/WindowSubSystem/UI.h>
#include <TArc/WindowSubSystem/ActionUtils.h>
#include "TArc/Utils/ReflectionHelpers.h"

#include <Engine/Engine.h>
#include <Engine/EngineContext.h>
#include <FileSystem/FileSystem.h>
#include <Base/ScopedPtr.h>
#include <Scene3D/Entity.h>
#include <Logger/Logger.h>
#include <Math/Vector.h>
#include <Render/Image/Image.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedObject.h>
#include <Reflection/ReflectionRegistrator.h>
#include <VisualScript/VisualScript.h>
#include <VisualScript/VisualScriptNode.h>

#include <QtGlobal>
#include <QMessageBox>

void InitVSResources()
{
    Q_INIT_RESOURCE(VisualScriptEditorResources);
    Q_INIT_RESOURCE(neresources);
}

namespace DAVA
{
class ImageTest
{
public:
    static Image* CreateRedImage()
    {
        static Image* image = nullptr;

        if (image == nullptr)
        {
            image = Image::Create(256, 256, PixelFormat::FORMAT_RGBA8888);
            for (uint32 pixelPos = 0; pixelPos < image->dataSize; pixelPos += 4)
            {
                image->data[pixelPos + 0] = 255; //r;
                image->data[pixelPos + 1] = 0; //g;
                image->data[pixelPos + 2] = 0; //b;
                image->data[pixelPos + 3] = 255; //a;
            }
        }

        return image;
    }

    static Image* CreateBlueImage()
    {
        static Image* image = nullptr;

        if (image == nullptr)
        {
            image = Image::Create(256, 512, PixelFormat::FORMAT_RGBA8888);
            for (uint32 pixelPos = 0; pixelPos < image->dataSize; pixelPos += 4)
            {
                image->data[pixelPos + 0] = 0; //r;
                image->data[pixelPos + 1] = 0; //g;
                image->data[pixelPos + 2] = 255; //b;
                image->data[pixelPos + 3] = 255; //a;
            }
        }

        return image;
    }

    DAVA_REFLECTION(ImageTest)
    {
        using namespace DAVA;
        ReflectionRegistrator<ImageTest>::Begin()
        .Method("CreateRedImage", &ImageTest::CreateRedImage)
        .Method("CreateBlueImage", &ImageTest::CreateBlueImage)
        .End();
    }
};

VisualScriptEditorModule::VisualScriptEditorModule()
{
    InitVSResources();

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptEditorDialogSettings);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptEditorData);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScriptEditorReflectionHolder);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisualScript);

    EmplaceTypeMeta<VisualScriptEditorDialogSettings>(M::HiddenField());

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Image);
    ImageTest::CreateRedImage();
    ImageTest::CreateBlueImage();
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ImageTest);
}

VisualScriptEditorModule::~VisualScriptEditorModule()
{
    //    if (dialog != nullptr)
    //    {
    //        delete dialog;
    //        dialog = nullptr;
    //    }
}

void VisualScriptEditorModule::PostInit()
{
    using namespace DAVA;

    QtAction* action = new QtAction(GetAccessor(), QString("Show Editor"));
    connections.AddConnection(action, &QAction::triggered, MakeFunction(this, &VisualScriptEditorModule::ShowEditor));

    ActionPlacementInfo placementInfo;
    placementInfo.AddPlacementPoint(CreateMenuPoint("Visual Script Editor"));

    GetUI()->AddAction(mainWindowKey, placementInfo, action);

    GetAccessor()->GetGlobalContext()->CreateData(std::make_unique<VisualScriptEditorData>());

    RegisterOperation(OpenVisualScriptFile.ID, this, &VisualScriptEditorModule::OpenVisualScriptFileHandler);
}

void VisualScriptEditorModule::OpenVisualScriptFileHandler(const QString& path)
{
    if (path.isEmpty() == false)
    {
        ShowEditor();
        DVASSERT(dialog != nullptr);
        if (GetEngineContext()->fileSystem->Exists(path.toStdString()))
        {
            dialog->OpenScriptByPath(path);
        }
    }
}

void VisualScriptEditorModule::ShowEditor()
{
    if (dialog == nullptr)
    {
        dialog = new VisualScriptEditorDialog(GetAccessor(), GetUI());
    }
    dialog->show();
}

DAVA_VIRTUAL_REFLECTION_IMPL(VisualScriptEditorModule)
{
    ReflectionRegistrator<VisualScriptEditorModule>::Begin()
    .ConstructorByPointer()
    .End();
}

IMPL_OPERATION_ID(OpenVisualScriptFile);

} //DAVA
