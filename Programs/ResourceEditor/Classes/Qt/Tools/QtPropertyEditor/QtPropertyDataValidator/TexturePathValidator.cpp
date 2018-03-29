#include "Classes/Qt/Tools/QtPropertyEditor/QtPropertyDataValidator/TexturePathValidator.h"

#include <REPlatform/DataNodes/Settings/RESettings.h>
#include <REPlatform/Global/GlobalOperations.h>
#include <REPlatform/Scene/Utils/RETextureDescriptorUtils.h>

#include <TArc/Core/Deprecated.h>

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
    DAVA::CommonInternalSettings* settings = DAVA::Deprecated::GetDataNode<DAVA::CommonInternalSettings>();
    if (v.type() == QVariant::String)
    {
        DAVA::FilePath texturePath = DAVA::FilePath(v.toString().toStdString());
        if (DAVA::FileSystem::Instance()->Exists(texturePath) && DAVA::RETextureDescriptorUtils::CreateOrUpdateDescriptor(texturePath))
        {
            DAVA::FilePath descriptorPath = DAVA::TextureDescriptor::GetDescriptorPathname(texturePath);
            v = QVariant(QString::fromStdString(descriptorPath.GetAbsolutePathname()));
        }
    }
}
