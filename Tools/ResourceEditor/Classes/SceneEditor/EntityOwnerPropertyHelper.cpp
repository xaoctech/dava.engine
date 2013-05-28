#include "EntityOwnerPropertyHelper.h"
#include "EditorSettings.h"

namespace DAVA {

const char* EntityOwnerPropertyHelper::SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME = "editor.designerName";
const char* EntityOwnerPropertyHelper::SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME = "editor.modificationData";

void EntityOwnerPropertyHelper::UpdateEntityOwner(KeyedArchive *customProperties)
{
	SetDesignerName(customProperties, EditorSettings::Instance()->GetDesignerName());
	UpdateModificationTime(customProperties);
}

void EntityOwnerPropertyHelper::SetDesignerName(KeyedArchive *customProperties, const String & name)
{
	customProperties->SetString(SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, name);
}

String EntityOwnerPropertyHelper::GetDesignerName(KeyedArchive *customProperties)
{
	return customProperties->GetString(SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, "nobody");
}

void EntityOwnerPropertyHelper::UpdateModificationTime(KeyedArchive *customProperties)
{
	time_t now = time(0);
    tm* utcTime = localtime(&now);
	
    String timeString = Format("%04d.%02d.%02d_%02d_%02d_%02d",
							   utcTime->tm_year + 1900, utcTime->tm_mon + 1, utcTime->tm_mday,
							   utcTime->tm_hour, utcTime->tm_min, utcTime->tm_sec);

	customProperties->SetString(SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, timeString);
}

String EntityOwnerPropertyHelper::GetModificationTime(KeyedArchive *customProperties)
{
	return customProperties->GetString(SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, "unknown");
}

};