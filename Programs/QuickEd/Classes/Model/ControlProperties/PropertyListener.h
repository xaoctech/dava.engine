#ifndef __QUICKED_PROPERTY_LISTENER_H__
#define __QUICKED_PROPERTY_LISTENER_H__

class AbstractProperty;
class RootProperty;
class ComponentPropertiesSection;
class VirtualPropertiesSection;
class StyleSheetProperty;
class StyleSheetPropertiesSection;
class StyleSheetSelectorProperty;
class StyleSheetSelectorsSection;

class PropertyListener
{
public:
    virtual ~PropertyListener() = 0;
    virtual void PropertyChanged(AbstractProperty* property)
    {
    }

    virtual void ComponentPropertiesWillBeAdded(RootProperty* root, ComponentPropertiesSection* section, int index)
    {
    }
    virtual void ComponentPropertiesWasAdded(RootProperty* root, ComponentPropertiesSection* section, int index)
    {
    }

    virtual void ComponentPropertiesWillBeRemoved(RootProperty* root, ComponentPropertiesSection* section, int index)
    {
    }
    virtual void ComponentPropertiesWasRemoved(RootProperty* root, ComponentPropertiesSection* section, int index)
    {
    }

    virtual void ComponentVirtualPropertiesWillBeAdded(RootProperty* root, VirtualPropertiesSection* section, int index)
    {
    }
    virtual void ComponentVirtualPropertiesWasAdded(RootProperty* root, VirtualPropertiesSection* section, int index)
    {
    }

    virtual void ComponenttVirtualPropertiesWillBeRemoved(RootProperty* root, VirtualPropertiesSection* section, int index)
    {
    }
    virtual void ComponenttVirtualPropertiesWasRemoved(RootProperty* root, VirtualPropertiesSection* section, int index)
    {
    }

    virtual void StylePropertyWillBeAdded(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
    {
    }
    virtual void StylePropertyWasAdded(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
    {
    }

    virtual void StylePropertyWillBeRemoved(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
    {
    }
    virtual void StylePropertyWasRemoved(StyleSheetPropertiesSection* section, StyleSheetProperty* property, int index)
    {
    }

    virtual void StyleSelectorWillBeAdded(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
    {
    }
    virtual void StyleSelectorWasAdded(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
    {
    }

    virtual void StyleSelectorWillBeRemoved(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
    {
    }
    virtual void StyleSelectorWasRemoved(StyleSheetSelectorsSection* section, StyleSheetSelectorProperty* property, int index)
    {
    }

    virtual void VirtualSectionRebuild(VirtualPropertiesSection* section, int oldCount, int newCount)
    {
    }
};

inline PropertyListener::~PropertyListener()
{
}

#endif // __QUICKED_PROPERTY_LISTENER_H__
