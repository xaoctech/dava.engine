#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushTextureSelector.h"
#include "REPlatform/Scene/Utils/ImageTools.h"

#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ComboBox.h>

#include <FileSystem/FilePath.h>
#include <FileSystem/FileList.h>
#include <Render/Image/ImageSystem.h>
#include <Render/Image/ImageConvert.h>
#include <Render/Texture.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
namespace BrushTextureSelectorDetail
{
QIcon ToIconCast(const Any& v)
{
    const BrushTextureSelector::BrushTexture& brush = v.Get<BrushTextureSelector::BrushTexture>();
    return brush.preview;
}

String ToStringCast(const Any& v)
{
    const BrushTextureSelector::BrushTexture& brush = v.Get<BrushTextureSelector::BrushTexture>();
    return brush.imageName;
}
}

BrushTextureSelector::BrushTextureSelector(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : TBase(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    if (castRegistered == false)
    {
        using namespace BrushTextureSelectorDetail;
        AnyCast<BrushTexture, QIcon>::Register(&ToIconCast);
        AnyCast<BrushTexture, String>::Register(&ToStringCast);
        castRegistered = true;
    }

    QtHBoxLayout* layout = new QtHBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);

    Reflection thisModel = Reflection::Create(ReflectedObject(this));

    ComboBox::Params p(params.accessor, params.ui, params.wndKey);
    p.fields[ComboBox::Fields::Value] = "value";
    p.fields[ComboBox::Fields::Enumerator] = "textures";
    ComboBox* combo = new ComboBox(p, wrappersProcessor, thisModel, this);
    layout->AddControl(combo);
    children.push_back(combo);

    FilePath brushTextures("~res:/ResourceEditor/LandscapeEditorV2/BrushTextures/");
    QString absPath = QString::fromStdString(brushTextures.GetAbsolutePathname());
    watcher.reset(new QFileSystemWatcher());
    watcher->addPath(absPath);
    connections.AddConnection(watcher.get(), &QFileSystemWatcher::directoryChanged, MakeFunction(this, &BrushTextureSelector::OnDirectoryChanged));
    OnDirectoryChanged(absPath);
    combo->ForceUpdate();
}

void BrushTextureSelector::ForceUpdate()
{
    for (ControlProxy* control : children)
    {
        control->ForceUpdate();
    }
}

void BrushTextureSelector::TearDown()
{
    for (ControlProxy* control : children)
    {
        control->TearDown();
    }
}

void BrushTextureSelector::UpdateControl(const ControlDescriptor& changedFields)
{
}

void BrushTextureSelector::OnDirectoryChanged(const QString& dir)
{
    Set<String> pathes;
    RefPtr<FileList> files(new FileList(FilePath(dir.toStdString()), false));
    for (uint32 i = 0; i < files->GetCount(); ++i)
    {
        if (files->IsDirectory(i) == false)
        {
            pathes.insert(files->GetPathname(i).GetAbsolutePathname());
        }
    }

    auto iter = textures.begin();
    while (iter != textures.end())
    {
        auto pathIter = pathes.find(iter->imagePath);
        if (pathIter == pathes.end())
        {
            iter = textures.erase(iter);
        }
        else
        {
            pathes.erase(pathIter);
            ++iter;
        }
    }

    for (const String& path : pathes)
    {
        BrushTexture brushTexture;
        brushTexture.SetImagePath(path);
        if (brushTexture.brush.Get() != nullptr)
        {
            textures.push_back(brushTexture);
        }
    }

    std::sort(textures.begin(), textures.end(), [](const BrushTexture& left, const BrushTexture& right) {
        return left.imageName < right.imageName;
    });
}

int32 BrushTextureSelector::GetBrush() const
{
    String texturePath = GetFieldValue(Fields::Value, String());

    for (int32 i = 0; i < static_cast<int32>(textures.size()); ++i)
    {
        if (textures[i].imagePath == texturePath)
        {
            return i;
        }
    }

    return -1;
}

void BrushTextureSelector::SetBrush(int32 brushIndex)
{
    wrapper.SetFieldValue(GetFieldName(Fields::Value), textures[brushIndex].imagePath);
}

void BrushTextureSelector::BrushTexture::SetImagePath(const String& imagePath)
{
    FilePath imgFilePath(imagePath);
    ImageInfo imgInfo = ImageSystem::GetImageInfo(imgFilePath);
    if (imgInfo.format == FORMAT_A8 || imgInfo.format == FORMAT_A16)
    {
        brush.Set(ImageSystem::LoadSingleMip(imgFilePath));
        preview = QIcon(QPixmap::fromImage(ImageTools::FromDavaImage(brush.Get())));
        this->imagePath = imagePath;
        imageName = imgFilePath.GetBasename();
    }
}

bool BrushTextureSelector::castRegistered = false;

DAVA_REFLECTION_IMPL(BrushTextureSelector)
{
    ReflectionRegistrator<BrushTextureSelector>::Begin()
    .Field("textures", &BrushTextureSelector::textures)
    .Field("value", &BrushTextureSelector::GetBrush, &BrushTextureSelector::SetBrush)
    .End();
}
} // namespace DAVA
