#ifndef __ENTITY_PROPERTY_CONTROL_H__
#define __ENTITY_PROPERTY_CONTROL_H__

#include "DAVAEngine.h"
using namespace DAVA;
#include "NodesPropertyControl.h"

class EntityPropertyControl : public NodesPropertyControl
{
public:
	EntityPropertyControl(const Rect & rect, bool createNodeProperties);
	virtual ~EntityPropertyControl();

	virtual void ReadFrom(Entity * entity);
};

#endif //__ENTITY_PROPERTY_CONTROL_H__