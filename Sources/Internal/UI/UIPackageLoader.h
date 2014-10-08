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
class UIPackageSectionLoader;
    
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
    virtual UIControl *CreateControlFromPrototype(UIControl *prototype, UIPackage *prototypePackage);
    virtual YamlNode *CreateYamlNode(UIControl *control);
    
protected:
    virtual void LoadPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport);
private:
    void LoadControlPropertiesFromYamlNode(UIControl *control, const InspInfo *typeInfo, const YamlNode *node, bool legacySupport);
    void LoadBgPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport);
    void LoadInternalControlPropertiesFromYamlNode(UIControl *control, const YamlNode *node, bool legacySupport);
    
protected:
    virtual UIPackageSectionLoader *CreateControlSectionLoader(UIControl *control, const String &name);
    virtual UIPackageSectionLoader *CreateBackgroundSectionLoader(UIControl *control, int bgNum);
    virtual UIPackageSectionLoader *CreateInternalControlSectionLoader(UIControl *control, int internalControlNum);
    virtual void ReleaseSectionLoader(UIPackageSectionLoader *section);
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
