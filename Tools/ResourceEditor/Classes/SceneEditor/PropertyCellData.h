/*
 *  PropertyValue.h
 *  SniperEditorMacOS
 *
 *  Created by Alexey Prosin on 12/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef PROPERTY_VALUE
#define PROPERTY_VALUE

#include "DAVAEngine.h"

using namespace DAVA;
class PropertyCell;
class PropertyCellData : public BaseObject
{
public:
    enum ValueType 
    {
        PROP_VALUE_STRING = 0,
        PROP_VALUE_INTEGER,
        PROP_VALUE_FLOAT,
        PROP_VALUE_BOOL,
        PROP_VALUE_MATRIX4,
        PROP_VALUE_COMBO_BOX,
        PROP_VALUE_SECTION,
        PROP_VALUE_MESSAGE,
        PROP_VALUE_COLOR,
        PROP_VALUE_SLIDER,
        PROP_VALUE_SUBSECTION,
        PROP_VALUE_TEXTUREPREVIEW,
        PROP_VALUE_DISTANCE,
        PROP_VALUE_FILEPATH,
        
        PROP_VALUE_COUNT
    };
    
    PropertyCellData(int _valueType);
    virtual ~PropertyCellData();

    int32 GetValueType();

    
    int32 GetInt();
    float32 GetFloat();
    const String& GetString();
    bool GetBool();
    const String& GetExtensionFilter();
    const Vector<String> & GetStringVector();
    int32 GetItemIndex(); 
    const Matrix4 & GetMatrix4() const;
    bool GetIsSectionOpened(); 
    int32 GetSectionElementsCount();
    const Message & GetMessage();
    bool GetClearDataEnabled();
    const Color &GetColor();
    float32 GetSliderValue();
    float32 GetSliderMinValue();
    float32 GetSliderMaxValue();
    float32 *GetDistances();
    int32 *GetTriangles();
    int32 GetDistancesCount();
    
    const FilePath & GetFilePath() const;
    
    Texture *GetTexture();

    void SetInt(int32 newInt);
    void SetFloat(float32 newFloat);
    void SetString(const String& newString);
    void SetBool(bool newBool);
    void SetExtensionFilter(const String& newString);
    void SetStringVector(const Vector<String> &newStrings);
    void SetItemIndex(int32 newItemIndex);
    void SetMatrix4(const Matrix4 & _matrix);
    void SetIsSectionOpened(bool isSectionOpened);
    void SetSectionElementsCount(int32 sectionElementsCount);
    void SetMessage(const Message &newMessage);
    void SetClearDataEnabled(bool enabled);
    void SetColor(const Color& newColor);
    void SetSliderValue(float32 newMin, float32 newMax, float32 newValue);
    void SetSliderValue(float32 newValue);
    void SetTexture(Texture *newTexture);
    void SetDistances(float32 *newDistances, int32 count);
    void SetTriangles(int32 *newTriangles, int32 count);
    void SetDistance(float32 newDistance, int32 index);
    void SetFilePath(const FilePath &pathname);
    
    int32 cellType;
    String key;
    bool isEditable;
    int index;
    
    PropertyCell *currentCell;
protected:
    int32 valueType;
    
    int32 intValue;
    float32 floatValue;
    String stringValue;
    bool boolValue;

    String extensionFilter;
    
    Vector<String> stringVector;

    Matrix4 matrix4;
    
    Message messageValue;
    
    Color color;
    
    float32 sliderValueMax;
    float32 sliderValueMin;
    float32 sliderValue;
    
    Texture *texture;
    
    float32 *distances;
    int32 *triangles;
    int32 distanceCount;
    
    FilePath filePath;
};

#endif