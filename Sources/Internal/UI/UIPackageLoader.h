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
    
class LegacyControlData : public BaseObject
{
public:
    struct Data {
        String name;
        bool isAggregator;
        Vector2 size;
    };
public:
    LegacyControlData(){}
    
    const Data *Get(const String &fwPath)
    {
        auto it = map.find(fwPath);
        return it == map.end() ? NULL : &(it->second);
    }
    
    void Put(const String &fwPath, const Data &data)
    {
        map[fwPath] = data;
    }
    
public:
    
    Map<String, Data> map;
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
    UIPackageLoader(LegacyControlData *data = NULL);
    virtual ~UIPackageLoader();

public:
    UIPackage *LoadPackage(const FilePath &packagePath);
    bool SavePackage(UIPackage *package);

protected:
    void SetUsingIntrospectionForLegacyData(bool useIntrospection);

private:
    void LoadRootControl(int index, UIPackage *currentPackage);
    UIControl *GetLoadedControlByName(const String &name, UIPackage *currentPackage);
    UIControl *CreateControl(const YamlNode *node, bool legacySupport, UIPackage *currentPackage);
    void LoadControl(UIControl *control, const YamlNode *node, bool legacySupport, UIPackage *currentPackage);

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
    LegacyControlData *legacyData;
    UIYamlLoader *yamlLoader;
    bool useIntrospectionForLegacyData;

    enum eItemStatus {
        STATUS_WAIT,
        STATUS_LOADING,
        STATUS_LOADED
    };
    
    struct QueueItem {
        String name;
        const YamlNode *node;
        UIControl *control;
        int status;
    };
    Vector<QueueItem> loadingQueue;
    DAVA::Map<DAVA::String, UIPackage*> importedPackages;
};

};

#endif // __DAVAENGINE_UI_PACKAGE_LOADER_H__
