#include "EntityPropertyControl.h"

EntityPropertyControl::EntityPropertyControl(const Rect & rect, bool createNodeProperties)
:	NodesPropertyControl(rect, createNodeProperties)
{

}

EntityPropertyControl::~EntityPropertyControl()
{

}

void EntityPropertyControl::ReadFrom(Entity * entity)
{
	propertyList->ReleaseProperties();

	int32 sectionsCount = entity->GetDataCount();
	for(int32 i = 0; i < sectionsCount; ++i)
	{
		propertyList->AddSection(entity->GetDataName(i), true);
	}

	
}
