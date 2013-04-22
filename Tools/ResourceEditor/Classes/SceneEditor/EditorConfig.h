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
    Vector<Color> colorListValues;
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
        PT_COLOR_LIST,

		PROPERTY_TYPES_COUNT
	};

    void ParseConfig(const FilePath &filePath);

	const Vector<String> & GetProjectPropertyNames();
	const Vector<String> & GetComboPropertyValues(const String & nameStr);
    const Vector<Color> & GetColorPropertyValues(const String& nameStr);

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
	Vector<String> empty;
    Vector<Color> emptyColors;
};



#endif // __EDITOR_CONFIG_H__