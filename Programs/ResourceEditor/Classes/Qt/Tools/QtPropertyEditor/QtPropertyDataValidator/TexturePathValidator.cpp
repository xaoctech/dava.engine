#include "Classes/Qt/Tools/QtPropertyEditor/QtPropertyDataValidator/TexturePathValidator.h"
#include "Classes/Application/RESettings.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Utils/TextureDescriptor/TextureDescriptorUtils.h"

#include <FileSystem/FileSystem.h>

TexturePathValidator::TexturePathValidator(const QStringList& value)
    : PathValidator(value)
{
}

bool TexturePathValidator::ValidateInternal(const QVariant& v)
{
    bool res = RegExpValidator::ValidateInternal(v);

    QString val = v.toString();
    if (res && val != "")
    {
        res = val.endsWith(QString::fromStdString(DAVA::TextureDescriptor::GetDescriptorExtension()));
    }

    return res;
}

void TexturePathValidator::FixupInternal(QVariant& v) const
{
    CommonInternalSettings* settings = REGlobal::GetGlobalContext()->GetData<CommonInternalSettings>();
    if (v.type() == QVariant::String)
    {
        DAVA::FilePath texturePath = DAVA::FilePath(v.toString().toStdString());
        if (DAVA::FileSystem::Instance()->Exists(texturePath) && TextureDescriptorUtils::CreateOrUpdateDescriptor(texturePath))
        {
            DAVA::FilePath descriptorPath = DAVA::TextureDescriptor::GetDescriptorPathname(texturePath);

            auto& texturesMap = DAVA::Texture::GetTextureMap();
            auto found = texturesMap.find(FILEPATH_MAP_KEY(descriptorPath));
            if (found != texturesMap.end())
            {
                found->second->ReloadAs(settings->textureViewGPU);
            }

            v = QVariant(QString::fromStdString(descriptorPath.GetAbsolutePathname()));
        }
    }
}
