#include "Entity/Entity.h"

namespace DAVA 
{

void Entity::AddComponent(Component * component)
{
    components.push_back(component);
}

};
