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

    enum eStringRepresentation          //data represent style for TYPE_STRING for storing in file
    {
        SR_AUTO_REPRESENTATION,         //YamlEmitter choose the data represent style automatically
        SR_PLAIN_REPRESENTATION,        //plain represent style
        SR_DOUBLE_QUOTED_REPRESENTATION,//data represent in double-quoted
    };

    enum eArrayRepresentation           //data represent style for TYPE_ARRAY for storing in file
    {
        AR_AUTO_REPRESENTATION,         //YamlEmitter choose the data represent style automatically
        AR_BLOCK_REPRESENTATION,        //data represent in multi-line with mark "- "
        AR_FLOW_REPRESENTATION,         //data represent one line in square brackets []
    };

    enum eMapRepresentation             //data represent style for TYPE_MAP for storing in file
    {
        MR_AUTO_REPRESENTATION,         //YamlEmitter choose the data represent style automatically
        MR_BLOCK_REPRESENTATION,        //data represent in multi-line
        MR_FLOW_REPRESENTATION,         //data represent one line in braces {}
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

    eType           GetType() const { return type; }
    int32           GetCount() const;

    void Print(int32 identation);
    void PrintToFile(DAVA::File* file, uint32 identationDepth = 0) const;

    bool            AsBool() const;
    int32           AsInt() const;//left for old code
    int32           AsInt32() const;
    uint32          AsUInt32() const;
    int64           AsInt64() const;
    uint64          AsUInt64() const;
    float32         AsFloat() const;
    const String    &AsString() const;
    const WideString &AsWString() const;
    const Vector<YamlNode*> &AsVector() const;
    const MultiMap<String, YamlNode*> &AsMap() const;
    FastName AsFastName() const;

    /*
        These functions work only if type of node is array
        All values must be integer or float to perform this conversion
     */
    Vector2         AsPoint() const;//Dizz: this one exists cause of Boroda
    Vector2         AsVector2() const;
    Vector3         AsVector3() const;
    Vector4         AsVector4() const;
    Color           AsColor() const;
    Rect            AsRect() const;
    VariantType     AsVariantType() const;

    const YamlNode *Get(const String & name) const;
    const YamlNode *Get(int32 index) const;
    const String   &GetItemKeyName(int32 index) const;

    // "Setters". These methods set data in string node.
    inline void     Set(bool value);
    inline void     Set(int32 value);
    inline void     Set(float32 value);
    inline void     Set(const char *value);
    inline void     Set(const String &value);
    inline void     Set(const WideString &value);

    // "Adders". These methods add data to array node.
    inline void     Add(bool value);
    inline void     Add(int32 value);
    inline void     Add(float32 value);
    inline void     Add(const char *value);
    inline void     Add(const String &value);
    inline void     Add(const WideString &value);
    inline void     Add(const Vector2 &value);
    inline void     Add(const Vector3 &value);
    inline void     Add(const Vector4 &value);
    inline void     Add(const VariantType &value);
    inline void     Add(YamlNode* value);

    // "Adders". These methods ADD node to the map, even in case the node with the same name is added.
    inline void     Add(const String& name, bool value);
    inline void     Add(const String& name, int32 value);
    inline void     Add(const String& name, float32 value);
    inline void     Add(const String& name, const char *value);
    inline void     Add(const String& name, const String &value);
    inline void     Add(const String& name, const WideString &value);
    inline void     Add(const String& name, const Vector2 &value);
    inline void     Add(const String& name, const Vector3 &value);
    inline void     Add(const String& name, const Vector4 &value);
    inline void     Add(const String& name, const VariantType &value);
    inline void     Add(const String& name, VariantType *varType);
    inline void     Add(const String &name, YamlNode *value);

    // "Setters". These methods REPLACE node in the map in case the node with the same name exists.
    inline void     Set(const String& name, bool value);
    inline void     Set(const String& name, int32 value);
    inline void     Set(const String& name, float32 value);
    inline void     Set(const String& name, const char *value);
    inline void     Set(const String& name, const String &value);
    inline void     Set(const String& name, const WideString &value);
    inline void     Set(const String& name, const Vector2 &value);
    inline void     Set(const String& name, const Vector3 &value);
    inline void     Set(const String& name, const Vector4 &value);
    inline void     Set(const String& name, const VariantType &value);
    inline void     Set(const String& name, VariantType *varType);
    inline void     Set(const String& name, YamlNode *value);

    // Specific adder for the whole node.
    inline void     AddNodeToMap(const String &name, YamlNode *value);
    // Specific setter for the whole node.
    inline void     SetNodeToMap(const String &name, YamlNode *value);
    // Setters for Map/Array nodes.
    inline void     AddNodeToArray(YamlNode* value);

    // Add the values to the current node of type Array.
    DAVA_DEPRECATED(inline void    AddValueToArray(int32 value));
    DAVA_DEPRECATED(inline void    AddValueToArray(float32 value));
    DAVA_DEPRECATED(inline void    AddValueToArray(const String& value));
    DAVA_DEPRECATED(inline void    AddValueToArray(const Vector2& value));
    DAVA_DEPRECATED(inline void    AddValueToArray(const Vector3& value));
    DAVA_DEPRECATED(inline void    AddValueToArray(const Vector4& value));
    DAVA_DEPRECATED(inline void    AddValueToArray(VariantType* value));

    // Remove node value from map
    void            RemoveNodeFromMap(const String & name);

protected:
    static YamlNode *CreateNodeWithVariantType(const VariantType &varType);
    static YamlNode *CreateNodeWithMatrix(const float32* array,uint32 dimension);
    static YamlNode *CreateNodeWithVector(const float32 array[],uint32 dimension);
    static YamlNode *CreateNodeWithByteArray(const uint8* byteArray,int32 byteArraySize);
    static YamlNode *CreateNodeWithKeyedArchive(KeyedArchive* archive);
    static YamlNode *CreateNodeWithString(const String &value, eStringRepresentation representation = SR_PLAIN_REPRESENTATION);
    static YamlNode *CreateNodeWithString(const WideString &value, eStringRepresentation representation = SR_PLAIN_REPRESENTATION);

    static YamlNode *CreateNodeFromVariantType(const VariantType &varType);

    bool            IsContainingMap() const;
    static String   FloatToCuttedString(float32 f);

    // Internal setters, which can both add or replace value in the map.
    void            InternalSetToString(const VariantType &varType);
    void            InternalAddToMap(const String& name, const VariantType &varType, bool rewritePreviousValue);
    void            InternalAddToArray(const VariantType &varType);

    void            InternalAddNodeToMap(const String& name, YamlNode* node, bool rewritePreviousValue);
    void            InternalAddNodeToArray(YamlNode* node);

    void            InternalSetString(const String &value);
    void            InternalSetWideString(const WideString &value);

private:
    int                         mapIndex;
    int                         mapCount;
    eType                       type;
    WideString                  stringValue;
    String                      nwStringValue;
    Vector<YamlNode*>           objectArray;
    MultiMap<String, YamlNode*> objectMap;
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

inline void YamlNode::Set(bool value)              { InternalSetToString(VariantType(value)); }
inline void YamlNode::Set(int32 value)             { InternalSetToString(VariantType(value)); }
inline void YamlNode::Set(float32 value)           { InternalSetToString(VariantType(value)); }
inline void YamlNode::Set(const char *value)       { InternalSetToString(VariantType((String)value)); }
inline void YamlNode::Set(const String &value)     { InternalSetToString(VariantType(value)); }
inline void YamlNode::Set(const WideString &value) { InternalSetToString(VariantType(value)); }

inline void YamlNode::Add(bool value)              { InternalAddToArray(VariantType(value)); }
inline void YamlNode::Add(int32 value)             { InternalAddToArray(VariantType(value)); }
inline void YamlNode::Add(float32 value)           { InternalAddToArray(VariantType(value)); }

inline void YamlNode::Add(const char *value)       { InternalAddToArray(VariantType((String)value)); }
inline void YamlNode::Add(const String &value)     { InternalAddToArray(VariantType(value)); }
inline void YamlNode::Add(const WideString &value) { InternalAddToArray(VariantType(value)); }

inline void YamlNode::Add(const Vector2 &value)    { InternalAddToArray(VariantType(value)); }
inline void YamlNode::Add(const Vector3 &value)    { InternalAddToArray(VariantType(value)); }
inline void YamlNode::Add(const Vector4 &value)    { InternalAddToArray(VariantType(value)); }

inline void YamlNode::Add(const VariantType &value){ InternalAddToArray(value); };
inline void YamlNode::Add(YamlNode* value)         { InternalAddNodeToArray(value); };

inline void YamlNode::Add(const String& name, bool value)              { InternalAddToMap(name, VariantType(value), false); }
inline void YamlNode::Add(const String& name, int32 value)             { InternalAddToMap(name, VariantType(value), false); }
inline void YamlNode::Add(const String& name, float32 value)           { InternalAddToMap(name, VariantType(value), false); }

inline void YamlNode::Add(const String& name, const char *value)       { InternalAddToMap(name, VariantType((String)value), false); }
inline void YamlNode::Add(const String& name, const String &value)     { InternalAddToMap(name, VariantType(value), false); }
inline void YamlNode::Add(const String& name, const WideString &value) { InternalAddToMap(name, VariantType(value), false); }

inline void YamlNode::Add(const String& name, const Vector2 &value)    { InternalAddToMap(name, VariantType(value), false); }
inline void YamlNode::Add(const String& name, const Vector3 &value)    { InternalAddToMap(name, VariantType(value), false); }
inline void YamlNode::Add(const String& name, const Vector4 &value)    { InternalAddToMap(name, VariantType(value), false); }

inline void YamlNode::Add(const String& name, const VariantType &value){ InternalAddToMap(name, value, false); }
inline void YamlNode::Add(const String& name, VariantType *varType)    { InternalAddToMap(name, *varType, false); }

inline void YamlNode::Add(const String &name, YamlNode *value)         { InternalAddNodeToMap(name, value, false);}

inline void YamlNode::Set(const String& name, bool value)              { InternalAddToMap(name, VariantType(value), true); }
inline void YamlNode::Set(const String& name, int32 value)             { InternalAddToMap(name, VariantType(value), true); }
inline void YamlNode::Set(const String& name, float32 value)           { InternalAddToMap(name, VariantType(value), true); }

inline void YamlNode::Set(const String& name, const char *value)       { InternalAddToMap(name, VariantType((String)value), true); }
inline void YamlNode::Set(const String& name, const String &value)     { InternalAddToMap(name, VariantType(value), true); }
inline void YamlNode::Set(const String& name, const WideString &value) { InternalAddToMap(name, VariantType(value), true); }

inline void YamlNode::Set(const String& name, const Vector2 &value)    { InternalAddToMap(name, VariantType(value), true); }
inline void YamlNode::Set(const String& name, const Vector3 &value)    { InternalAddToMap(name, VariantType(value), true); }
inline void YamlNode::Set(const String& name, const Vector4 &value)    { InternalAddToMap(name, VariantType(value), true); }

inline void YamlNode::Set(const String& name, const VariantType &value){ InternalAddToMap(name, value, true); }
inline void YamlNode::Set(const String& name, VariantType *varType)    { InternalAddToMap(name, *varType, true); }

inline void YamlNode::Set(const String& name, YamlNode *value)         { InternalAddNodeToMap(name, value, true); }

inline void YamlNode::AddNodeToMap(const String &name, YamlNode *value){ Add(name, value);}
inline void YamlNode::SetNodeToMap(const String &name, YamlNode *value){ Set(name, value); }
inline void YamlNode::AddNodeToArray(YamlNode* value){ Add(value); }

inline void YamlNode::AddValueToArray(int32 value)                      { Add(value); };
inline void YamlNode::AddValueToArray(float32 value)                    { Add(value); };
inline void YamlNode::AddValueToArray(const String& value)              { Add(value); };
inline void YamlNode::AddValueToArray(const Vector2& value)             { Add(value); };
inline void YamlNode::AddValueToArray(const Vector3& value)             { Add(value); };
inline void YamlNode::AddValueToArray(const Vector4& value)             { Add(value); };
inline void YamlNode::AddValueToArray(VariantType* value)               { Add(*value);};
};

#endif // __DAVAENGINE_YAML_NODE_H__