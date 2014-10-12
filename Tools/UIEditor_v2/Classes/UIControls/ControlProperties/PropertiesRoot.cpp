#include "PropertiesRoot.h"

#include "UIControls/PropertiesSection.h"

using namespace DAVA;

PropertiesRoot::PropertiesRoot()
{
    
}

PropertiesRoot::~PropertiesRoot()
{
    for (auto it = children.begin(); it != children.end(); ++it)
    {
        (*it)->SetParent(NULL);
        (*it)->Release();
    }
    children.clear();
}

void PropertiesRoot::AddProperty(PropertiesSection *section)
{
    DVASSERT(section->GetParent() == NULL);
    section->SetParent(this);
    children.push_back(SafeRetain(section));
}

int PropertiesRoot::GetCount() const
{
    return (int) children.size();
}

BaseProperty *PropertiesRoot::GetProperty(int index) const
{
    return children[index];
}

String PropertiesRoot::GetName() const {
    return "ROOT";
}

BaseProperty::ePropertyType PropertiesRoot::GetType() const {
    return TYPE_HEADER;
}
