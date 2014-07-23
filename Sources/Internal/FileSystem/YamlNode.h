/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __DAVAENGINE_YAML_NODE_H__
#define __DAVAENGINE_YAML_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Base/FastName.h"

namespace DAVA
{
class KeyedArchive;
class VariantType;
/**
    \ingroup yaml
    \brief this class is base yaml node that is used for everything connected with yaml
*/
class YamlNode : public BaseObject
{
public:
    enum eType
    {
        TYPE_STRING = 0,
        TYPE_ARRAY,
        TYPE_MAP,
    };

    enum eStringRepresentation//for TYPE_STRING
    {
        SR_AUTO_REPRESENTATION,
        SR_PLAIN_REPRESENTATION,
        SR_DOUBLE_QUOTED_REPRESENTATION,
    };

    enum eArrayRepresentation//for TYPE_ARRAY
    {
        AR_AUTO_REPRESENTATION,
        AR_BLOCK_REPRESENTATION,
        AR_FLOW_REPRESENTATION
    };

    enum eMapRepresentation//for TYPE_MAP
    {
        MR_AUTO_REPRESENTATION,
        MR_BLOCK_REPRESENTATION,
        MR_FLOW_REPRESENTATION
    };

    // Predefined node name to store Relative Depth.
    static const char8* SAVE_INDEX_NAME;

protected:
    virtual ~YamlNode();
public:
    YamlNode(eType type);
    static YamlNode *CreateStringNode(eStringRepresentation representation = SR_PLAIN_REPRESENTATION);
    static YamlNode *CreateArrayNode(eArrayRepresentation representation = AR_FLOW_REPRESENTATION);
    static YamlNode *CreateMapNode(eMapRepresentation valRepresentation = MR_BLOCK_REPRESENTATION, eStringRepresentation keyRepresentation = SR_PLAIN_REPRESENTATION);

    void Print(int32 identation);
    void PrintToFile(DAVA::File* file, uint32 identationDepth = 0) const;

    bool			AsBool() const;
    int32			AsInt() const;//left for old code
    int32			AsInt32() const;
    uint32			AsUInt32() const;
    int64			AsInt64() const;
    uint64			AsUInt64() const;
    float32			AsFloat() const;
    const String &	AsString() const;
    const WideString & AsWString() const;
    const Vector<YamlNode*> & AsVector() const;
    const MultiMap<String, YamlNode*> & AsMap() const;
    FastName AsFastName() const;

    /*
        These functions work only if type of node is array
        All values must be integer or float to perform this conversion
     */
    Vector2			AsPoint() const;//Dizz: this one exists cause of Boroda
    Vector2			AsVector2() const;
    Vector3			AsVector3() const;
    Vector4			AsVector4() const;
    Color			AsColor() const;
    Rect			AsRect() const;
    VariantType     AsVariantType() const;

    const YamlNode *		Get(const String & name) const;
    const YamlNode *		Get(int32 index) const;
    const String &	GetItemKeyName(int32 index) const;

    // "Adders". These methods ADD node to the map, even in case the node with the same name is added.
    void            Add(const String& name, bool value);
    void            Add(const String& name, int32 value);
    void            Add(const String& name, float32 value);

    void            Add(const String& name, const char8* value);
    void            Add(const String& name, const String& value);
    void            Add(const String& name, const WideString& value);

    void            Add(const String& name, const Vector2& value);
    void            Add(const String& name, const Vector3& value);
    void            Add(const String& name, const Vector4& value);

    void            Add(const String& name, VariantType* varType);

    // Specific adder for the whole node.
    void            AddNodeToMap(const String& name, YamlNode* node);


    // "Setters". These methods REPLACE node in the map in case the node with the same name exists.
    void            Set(const String& name, bool value);
    void            Set(const String& name, int32 value);
    void            Set(const String& name, float32 value);

    void            Set(const String& name, const char8* value);
    void            Set(const String& name, const String& value);
    void            Set(const String& name, const WideString& value);

    void            Set(const String& name, const Vector2& value);
    void            Set(const String& name, const Vector3& value);
    void            Set(const String& name, const Vector4& value);

    void            Set(const String& name, VariantType* varType);

    // Specific setter for the whole node.
    void            SetNodeToMap(const String& name, YamlNode* node);

    // Setters for Map/Array nodes.
    void            AddNodeToArray(YamlNode* node);

    // Add the values to the current node of type Array.
    void            AddValueToArray(int32 value);
    void            AddValueToArray(float32 value);
    void            AddValueToArray(const String& value);
    void            AddValueToArray(const Vector2& value);
    void            AddValueToArray(const Vector3& value);
    void            AddValueToArray(const Vector4& value);
    void            AddValueToArray(VariantType* value);

    // Remove node value from map
    void            RemoveNodeFromMap(const String & name);

    eType			GetType() const { return type; }
    int32			GetCount() const;
    bool			IsWideString() const { return isWideString; };

    void            InitFromKeyedArchive(KeyedArchive* archive);
    void            InitFromVariantType(VariantType* varType);

protected:
    void            FillContentAccordingToVariantTypeValue(VariantType* varType);

    void            ProcessMatrix(const float32* array,uint32 dimension);
    void            ProcessVector(const float32 array[],uint32 dimension);
    void            ProcessByteArray(const uint8* byteArray,int32 byteArraySize);
    void            ProcessKeyedArchive(KeyedArchive* archive);

    bool            IsContainingMap() const;
    String          FloatToCuttedString(float32 f);

    // Internal setters, which can both add or replace value in the map.
    void            InternalSet(const String& name, bool value, bool rewritePreviousValue);
    void            InternalSet(const String& name, int32 value, bool rewritePreviousValue);
    void            InternalSet(const String& name, float32 value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const char8* value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const String& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const WideString& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const Vector2& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const Vector3& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, const Vector4& value, bool rewritePreviousValue);
    void            InternalSet(const String& name, VariantType* varType, bool rewritePreviousValue);

    void            InternalAddNodeToArray(YamlNode* node);
    void            InternalSetNodeToMap(const String& name, YamlNode* node, bool rewritePreviousValue);
    void            InternalSetString(const String &value);
    void            InternalSetWideString(const WideString &value);
private:

    int						mapIndex;
    int						mapCount;
    eType					type;
    WideString				stringValue;
    String					 nwStringValue;
    Vector<YamlNode*>		 objectArray;
    MultiMap<String, YamlNode*>	objectMap;
    bool					isWideString;
    union
    {
        eStringRepresentation stringStyle;
        eArrayRepresentation arrayStyle;
        struct
        {
            eMapRepresentation value;
            eStringRepresentation key;
        }mapStyle;
    }representation;

    friend class YamlParser;
    friend class YamlEmitter;
};
};

#endif // __DAVAENGINE_YAML_NODE_H__