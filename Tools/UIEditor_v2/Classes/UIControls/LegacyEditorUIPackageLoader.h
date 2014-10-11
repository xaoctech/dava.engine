
#ifndef __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__
#define __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__

#include "DAVAEngine.h"

class LegacyControlData : public DAVA::BaseObject
{
public:
    struct Data {
        DAVA::String name;
        bool isAggregator;
        DAVA::Vector2 size;
    };
public:
    LegacyControlData(){}
    
    const Data *Get(const DAVA::String &fwPath)
    {
        auto it = map.find(fwPath);
        return it == map.end() ? NULL : &(it->second);
    }
    
    void Put(const DAVA::String &fwPath, const Data &data)
    {
        map[fwPath] = data;
    }
    
public:
    
    DAVA::Map<DAVA::String, Data> map;
};

class LegacyEditorUIPackageLoader
{
public:
    LegacyEditorUIPackageLoader(LegacyControlData *data = NULL);
    virtual ~LegacyEditorUIPackageLoader();
    
public:
    DAVA::UIPackage *LoadPackage(const DAVA::FilePath &packagePath);
    
private:
    DAVA::UIControl *CreateControl(const DAVA::YamlNode *node, DAVA::UIPackage *currentPackage);
    void LoadControl(DAVA::UIControl *control, const DAVA::YamlNode *node, DAVA::UIPackage *currentPackage);
    
protected:
    virtual DAVA::UIControl *CreateControlByClassName(const DAVA::String &className);
    virtual DAVA::UIControl *CreateCustomControl(const DAVA::String &customClassName, const DAVA::String &baseClassName);
    virtual DAVA::UIControl *CreateControlFromPrototype(DAVA::UIControl *prototype);
    
protected:
    virtual void LoadPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node);
private:
    void LoadControlPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::InspInfo *typeInfo, const DAVA::YamlNode *node);
    void LoadBgPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node);
    void LoadInternalControlPropertiesFromYamlNode(DAVA::UIControl *control, const DAVA::YamlNode *node);
    
protected:
    virtual DAVA::UIPackageSectionLoader *CreateControlSectionLoader(DAVA::UIControl *control, const DAVA::String &name);
    virtual DAVA::UIPackageSectionLoader *CreateBackgroundSectionLoader(DAVA::UIControl *control, int bgNum);
    virtual DAVA::UIPackageSectionLoader *CreateInternalControlSectionLoader(DAVA::UIControl *control, int internalControlNum);
    virtual void ReleaseSectionLoader(DAVA::UIPackageSectionLoader *section);
    virtual DAVA::VariantType ReadVariantTypeFromYamlNode(const DAVA::InspMember *member, const DAVA::YamlNode *node, int subNodeIndex, const DAVA::String &propertyName);
    
private:
    DAVA::String GetOldPropertyName(const DAVA::String controlClassName, const DAVA::String name);
    DAVA::String GetOldBgPrefix(const DAVA::String controlClassName, const DAVA::String name);
    DAVA::String GetOldBgPostfix(const DAVA::String controlClassName, const DAVA::String name);
    
private:
    DAVA::Map<DAVA::String, DAVA::Map<DAVA::String, DAVA::String> > propertyNamesMap;
    DAVA::Map<DAVA::String, DAVA::String> baseClasses;
    
private:
    LegacyControlData *legacyData;
    DAVA::UIYamlLoader *yamlLoader;
    
    DAVA::Map<DAVA::String, DAVA::UIPackage*> importedPackages;
};


#endif // __UI_LEGACY_EDITOR_PACKAGE_LOADER_H__
