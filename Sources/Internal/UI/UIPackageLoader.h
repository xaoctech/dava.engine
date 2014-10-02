#ifndef __DAVAENGINE_UI_PACKAGE_LOADER_H__
#define __DAVAENGINE_UI_PACKAGE_LOADER_H__
#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace DAVA
{
class UIYamlLoader;
class UIControl;
class YamlNode;
class FilePath;
class UIPackage;
class UIControlFactory;
class UIControlBackground;
    
class LegacyControlData
{
public:
    LegacyControlData(){}
    
public:
    String name;
    bool isAggregator;
    Vector2 size;
};
    
////////////////////////////////////////////////////////////////////////////////
// UIPackageSectionIControlSection
////////////////////////////////////////////////////////////////////////////////
    
class UIPackageSection : public BaseObject
{
public:
    UIPackageSection();
    virtual ~UIPackageSection();
    
    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value) = 0;
    virtual BaseObject *GetBaseObject() const = 0;
    virtual String GetName() const = 0;
    
    virtual void Apply() = 0;
};
    
class UIPackageControlSection : public UIPackageSection
{
public:
    UIPackageControlSection(UIControl *control, const String &name);
    virtual ~UIPackageControlSection();
    
    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value);
    virtual BaseObject *GetBaseObject() const;
    virtual String GetName() const;
    
    virtual void Apply();

protected:
    String name;
    UIControl *control;
};

class UIPackageBackgroundSection : public UIPackageSection
{
public:
    UIPackageBackgroundSection(UIControl *control, int num);
    virtual ~UIPackageBackgroundSection();
    
    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value);
    virtual BaseObject *GetBaseObject() const;
    virtual String GetName() const;
    
    virtual void Apply();

protected:
    UIControl *control;
    UIControlBackground *bg;
    bool bgWasCreated;
    bool bgHasChanges;
    int bgNum;
};

class UIPackageInternalControlSection : public UIPackageSection
{
public:
    UIPackageInternalControlSection(UIControl *control, int num);
    virtual ~UIPackageInternalControlSection();
    
    virtual void SetProperty(const InspMember *member, const DAVA::VariantType &value);
    virtual BaseObject *GetBaseObject() const;
    virtual String GetName() const;

    virtual void Apply();

protected:
    UIControl *control;
    UIControl *internalControl;
    bool internalWasCreated;
    bool internalHasChanges;
    int internalControlNum;
};

////////////////////////////////////////////////////////////////////////////////
// UIPackageLoader
////////////////////////////////////////////////////////////////////////////////
class UIPackageLoader
{
public:
    UIPackageLoader();
    virtual ~UIPackageLoader();

public:
    UIPackage *LoadPackage(const FilePath &packagePath, const LegacyControlData *data = NULL);
    bool SavePackage(UIPackage *package);

protected:
    void SetUsingIntrospectionForLegacyData(bool useIntrospection);

private:
    UIControl *CreateControl(const YamlNode *node, const UIPackage *package, bool legacySupport);
    void LoadControl(UIControl *control, const YamlNode *node, const UIPackage *package, bool legacySupport);

protected:
    virtual UIControl *CreateControlByClassName(const String &className);
    virtual UIControl *CreateCustomControl(const String &customClassName, const String &baseClassName);
    virtual UIControl *CreateControlFromPrototype(UIControl *prototype);
    virtual YamlNode *CreateYamlNode(UIControl *control);
    
protected:
    virtual void LoadPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport);
private:
    void LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node, bool legacySupport);
    void LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport);
    void LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport);
    
protected:
    virtual UIPackageSection *CreateControlSection(UIControl *control, const String &name);
    virtual UIPackageSection *CreateBackgroundSection(UIControl *control, int bgNum);
    virtual UIPackageSection *CreateInternalControlSection(UIControl *control, int internalControlNum);
    virtual void ReleaseSection(UIPackageSection *section);
    virtual VariantType ReadVariantTypeFromYamlNode(const InspMember *member, const YamlNode *node, int subNodeIndex, const String &propertyName, bool legacySupport);

    virtual bool AddObjectPropertyToYamlNode(BaseObject *obj, const InspMember *member, YamlNode *node);
    
private:
    void AddPropertiesToNode(UIControl *control, YamlNode *node);
    DAVA::String GetOldPropertyName(const DAVA::String controlClassName, const DAVA::String name);
    DAVA::String GetOldBgPrefix(const DAVA::String controlClassName, const DAVA::String name);
    DAVA::String GetOldBgPostfix(const DAVA::String controlClassName, const DAVA::String name);
    
private:
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::String> > propertyNamesMap;
    DAVA::Map<DAVA::String, DAVA::String> baseClasses;

private:
    UIYamlLoader *yamlLoader;
    bool useIntrospectionForLegacyData;

};

};

#endif // __DAVAENGINE_UI_PACKAGE_LOADER_H__
