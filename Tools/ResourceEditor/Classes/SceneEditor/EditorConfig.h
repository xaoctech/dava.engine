#ifndef __EDITOR_CONFIG_H__
#define __EDITOR_CONFIG_H__

#include "DAVAEngine.h"

using namespace DAVA;

class PropertyList;

class PropertyDescription : public BaseObject
{
public:
	PropertyDescription() : BaseObject(), type(0) {};

	String name;
	int32 type;
	VariantType defaultValue;
	Vector<String> comboValues;
};

class EditorConfig: public Singleton<EditorConfig>
{
    
public:
    EditorConfig();
    virtual ~EditorConfig();

	enum ePropertyType
	{
		PT_NONE = 0,
		PT_BOOL,
		PT_INT,
		PT_FLOAT,
		PT_STRING,
		PT_COMBOBOX,

		PROPERTY_TYPES_COUNT
	};

    void ParseConfig(const String &filePath);

	const Vector<String> &GetProjectPropertyNames();

	bool HasProperty(const String &propertyName);
	int32 GetPropertyValueType(const String &propertyName);
	VariantType *GetPropertyDefaultValue(const String &propertyName);

	void AddPropertyEditor(PropertyList *propertyList, const String &propertyName, VariantType *propertyValue);

protected:
	void ClearConfig();

	PropertyDescription* GetPropertyDescription(const String &propertyName);

	int32 GetValueTypeFromPropertyType(int32 propertyType);
	int32 ParseType(const String &typeStr);

	Vector<String> propertyNames;
	Map<String, PropertyDescription*> properties;
};



#endif // __EDITOR_CONFIG_H__