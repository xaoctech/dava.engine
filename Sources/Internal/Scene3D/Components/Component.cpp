#include "Scene3D/Components/Component.h"
#include "Debug/DVAssert.h"

namespace DAVA
{

uint32 Component::GetType()
{
    //DVASSERT("Zero type component" && 0);
    return COMPONENT_COUNT;
};


}